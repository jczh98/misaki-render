#include <array>
#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/film.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <aspirin/thread.h>
#include <aspirin/utils.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace aspirin {

class SPPMIntegrator : public Integrator {
public:
    struct SPPMPixel {

        Float radius = 0;
        Spectrum value;

        struct VisiblePoint {
            VisiblePoint() {}
            VisiblePoint(const Vector3 &p, const Vector3 &wi, const BSDF *bsdf,
                         const Spectrum &beta)
                : p(p), wi(wi), bsdf(bsdf), beta(beta) {}
            Vector3 p;
            Vector3 wi;
            const BSDF *bsdf = nullptr;
            Spectrum beta;
        } vp;

        AtomicFloat<Float> phi[3];
        std::atomic<int> m;
        float n = 0;
        Spectrum tau;
    };

    struct SPPMPixelListNode {
        SPPMPixel *pixel;
        SPPMPixelListNode *next;
    };

    inline unsigned int hash(const Vector3i &p, int hash_size) {
        return (unsigned int) ((p.x() * 73856093) ^ (p.y() * 19349663) ^
                               (p.z() * 83492791)) %
               hash_size;
    }

    static bool to_grid(const Vector3 &p, const BoundingBox3 &bounds,
                        const int grid_res[3], Vector3i *pi) {
        bool in_bounds = true;
        Vector3 pg     = bounds.offset(p);
        for (int i = 0; i < 3; ++i) {
            (*pi)[i] = (int) (grid_res[i] * pg[i]);
            in_bounds &= ((*pi)[i] >= 0 && (*pi)[i] < grid_res[i]);
            (*pi)[i] = std::clamp<int>((*pi)[i], 0, grid_res[i] - 1);
        }
        return in_bounds;
    }

    SPPMIntegrator(const Properties &props) : Integrator(props) {}

    bool render(Scene *scene, Sensor *sensor) {
        auto film        = sensor->film();
        auto film_size   = film->size();
        auto pixel_count = film_size.prod();
        std::unique_ptr<SPPMPixel[]> pixels(new SPPMPixel[pixel_count]);
        for (uint32_t i = 0; i < pixel_count; i++) {
            pixels[i].radius = m_initial_radius;
        }
        const Float inv_sqrt_spp = 1.f / std::sqrt(m_iterations);
        for (int iter = 0; iter < m_iterations; iter++) {
            int m_block_size = APR_BLOCK_SIZE;
            BlockGenerator gen(film_size, Vector2i::Zero(), m_block_size);
            size_t total_blocks = gen.block_count();
            Timer timer;
            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, total_blocks, 1),
                [&](const tbb::blocked_range<size_t> &range) {
                    auto sampler          = sensor->sampler()->clone();
                    ref<ImageBlock> block = new ImageBlock(
                        Vector2i::Constant(m_block_size), film->filter());
                    for (auto i = range.begin(); i != range.end(); ++i) {
                        auto [offset, size, block_id] = gen.next_block();
                        block->set_offset(offset);
                        block->set_size(size);
                        for (int y = 0; y < size.y(); ++y) {
                            for (int x = 0; x < size.x(); ++x) {
                                Vector2 pos = Vector2(x, y);
                                if (pos.x() >= size.x() || pos.y() >= size.y())
                                    continue;
                                pos = pos + offset.template cast<Float>();
                                auto position_sample = pos + sampler->next2d();
                                auto [ray, beta] =
                                    sensor->sample_ray_differential(
                                        position_sample, sampler->next2d());
                                ray.scale_differential(inv_sqrt_spp);
                                SPPMPixel &pixel =
                                    pixels[pos.x() + pos.y() * film_size.x()];
                                bool is_specular = false;
                                SurfaceInteraction si =
                                    scene->ray_intersect(ray);
                                Float eta = 1.f;
                                for (int depth = 0; depth < m_max_depth;
                                     depth++) {
                                    if (!si.is_valid()) {
                                        if (scene->environment() != nullptr)
                                            pixel.value +=
                                                beta *
                                                scene->environment()->eval(si);
                                        break;
                                    }
                                    auto emitter = si.shape->emitter();
                                    // Compute emitted radiance
                                    if (emitter != nullptr &&
                                        (depth == 0 || is_specular)) {
                                        pixel.value += beta * emitter->eval(si);
                                    }
                                    /*
                                     * Direct illumination sampling
                                     */
                                    BSDFContext ctx;
                                    auto bsdf = si.bsdf(ray);
                                    if (has_flag(bsdf->flags(),
                                                 BSDFFlags::Smooth)) {
                                        auto [ds, emitter_val] =
                                            scene->sample_emitter_direction(
                                                si, sampler->next2d(), true);
                                        if (ds.pdf != 0.f) {
                                            auto wo = si.to_local(ds.d);
                                            Spectrum bsdf_val =
                                                bsdf->eval(ctx, si, wo);
                                            Float bsdf_pdf =
                                                bsdf->pdf(ctx, si, wo);
                                            pixel.value +=
                                                beta * emitter_val * bsdf_val;
                                        }
                                    }
                                    if (has_flag(bsdf->flags(),
                                                 BSDFFlags::Diffuse) ||
                                        (has_flag(bsdf->flags(),
                                                  BSDFFlags::Glossy) &&
                                         depth == m_max_depth - 1)) {
                                        pixel.vp = { si.p, -ray.d, bsdf, beta };
                                        break;
                                    }
                                    if (depth < m_max_depth - 1) {
                                        auto [bs, bsdf_val] = bsdf->sample(
                                            ctx, si, sampler->next1d(),
                                            sampler->next2d());
                                        if (bs.pdf == 0. || is_black(bsdf_val))
                                            break;
                                        is_specular = has_flag(
                                            bs.sampled_type, BSDFFlags::Delta);
                                        const auto wo = si.to_world(bs.wo);
                                        beta *= bsdf_val;
                                        eta *= bs.eta;
                                        Float q = std::min(beta.maxCoeff() *
                                                               eta * eta,
                                                           Float(0.95));
                                        if (sampler->next1d() >= q)
                                            break;
                                        beta /= q;

                                        ray = si.spawn_ray(wo);
                                        si  = scene->ray_intersect(ray);
                                    }
                                }
                            }
                        }
                    }
                });
            int grid_res[3];
            BoundingBox3 grid_bounds;
            const int hash_size = pixel_count;
            std::vector<std::atomic<SPPMPixelListNode *>> grid(hash_size);
            Float max_radius = 0.;
            for (int i = 0; i < pixel_count; i++) {
                const SPPMPixel &pixel = pixels[i];
                if (is_black(pixel.vp.beta))
                    continue;
                grid_bounds.expand(
                    BoundingBox3(pixel.vp.p - Vector3::Constant(pixel.radius),
                                 pixel.vp.p + Vector3::Constant(pixel.radius)));
                max_radius = std::max(max_radius, pixel.radius);
            }
            Vector3 diag      = grid_bounds.diagonal();
            int base_grid_res = (int) (diag.maxCoeff() / max_radius);
            for (int i = 0; i < 3; ++i)
                grid_res[i] = std::max(
                    (int) (base_grid_res * diag[i] / diag.maxCoeff()), 1);
            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, pixel_count, 1),
                [&](const tbb::blocked_range<size_t> &range) {
                    for (auto i = range.begin(); i != range.end(); ++i) {
                        SPPMPixel &pixel = pixels[i];
                        if (!is_black(pixel.vp.beta)) {
                            Vector3i pmin, pmax;
                            to_grid(pixel.vp.p -
                                        Vector3::Constant(pixel.radius),
                                    grid_bounds, grid_res, &pmin);
                            to_grid(pixel.vp.p +
                                        Vector3::Constant(pixel.radius),
                                    grid_bounds, grid_res, &pmax);
                            for (int z = pmin.z(); z <= pmax.z(); z++) {
                                for (int y = pmin.y(); y <= pmax.y(); y++) {
                                    for (int x = pmin.x(); x <= pmax.x(); x++) {
                                        int h =
                                            hash(Vector3i(x, y, z), hash_size);
                                        auto *node =
                                            new SPPMPixelListNode();
                                        node->pixel = &pixel;
                                        node->next  = grid[h];
                                        while (!grid[h].compare_exchange_weak(
                                            node->next, node))
                                            ;
                                    }
                                }
                            }
                        }
                    }
                });
            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, m_photons, 1),
                [&](const tbb::blocked_range<size_t> &range) {
                    auto sampler = sensor->sampler()->clone();
                    for (int i = range.begin(); i < range.end(); i++) {
                        auto emitter_sel_pdf = 1.f / scene->emitters().size();
                        auto emitter_sample  = sampler->next1d();
                        auto index =
                            std::min(uint32_t(emitter_sample *
                                              (Float) scene->emitters().size()),
                                     (uint32_t) scene->emitters().size() - 1);
                        const auto emitter = scene->emitters()[index];
                        auto [ray, flux]   = emitter->sample_ray(
                            sampler->next2d(), sampler->next2d());
                        flux /= emitter_sel_pdf * 10000;
                        SurfaceInteraction si = scene->ray_intersect(ray);
                        for (int depth = 0; depth < m_max_depth; depth++) {
                            if (!si.is_valid())
                                break;
                            BSDFContext ctx;
                            if (depth > 0) {
                                Vector3i photon_grid_index;
                                if (to_grid(si.p, grid_bounds, grid_res,
                                            &photon_grid_index)) {
                                    int h = hash(photon_grid_index, hash_size);
                                    for (SPPMPixelListNode *node = grid[h].load(
                                             std::memory_order_relaxed);
                                         node != nullptr; node = node->next) {
                                        SPPMPixel &pixel = *node->pixel;
                                        Float radius     = pixel.radius;
                                        if ((pixel.vp.p - si.p).squaredNorm() >
                                            radius * radius)
                                            continue;
                                        SurfaceInteraction si_bsdf = si;
                                        si_bsdf.wi = si.to_local(pixel.vp.wi);
                                        Spectrum phi =
                                            flux * pixel.vp.bsdf->eval(ctx, si_bsdf, si.wi);
                                        for (int channel = 0; channel < 3;
                                             channel++) {
                                            pixel.phi[channel].add(phi[channel]);
                                        }
                                        ++pixel.m;
                                    }
                                }
                            }
                            auto bsdf           = si.bsdf(ray);
                            auto [bs, bsdf_val] = bsdf->sample(
                                ctx, si, sampler->next1d(), sampler->next2d());
                            if (bs.pdf == 0.f || is_black(bsdf_val))
                                break;
                            Spectrum bnew = flux * bsdf_val / bs.pdf;
                            Float q = std::min(bnew.maxCoeff(), Float(0.95));
                            if (sampler->next1d() >= q)
                                break;
                            flux = bnew / q;
                            ray  = si.spawn_ray(si.to_world(bs.wo));
                            si   = scene->ray_intersect(ray);
                        }
                    }
                });
            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, pixel_count, 1),
                [&](const tbb::blocked_range<size_t> &range) {
                    for (int i = range.begin(); i < range.end(); i++) {
                        SPPMPixel &p = pixels[i];
                        if (p.m > 0) {
                            Float gamma = 2.0 / 3;
                            Float nnew  = p.n + gamma * p.m;
                            Float rnew =
                                p.radius * std::sqrt(nnew / (p.n + p.m));
                            Spectrum phi;
                            for (int channel = 0; channel < 3; channel++) {
                                phi[channel] = p.phi[channel];
                            }
                            p.tau = (p.tau + p.vp.beta * phi) * (rnew * rnew) /
                                    (p.radius * p.radius);
                            p.n      = nnew;
                            p.radius = rnew;
                            p.m      = 0;
                        }
                        p.vp.beta = Spectrum::Zero();
                        p.vp.bsdf = nullptr;
                    }
                });
            if (iter == m_iterations - 1 ||
                ((iter + 1) % m_develop_frequency == 0)) {
                ref<ImageBlock> block =
                    new ImageBlock(film_size, film->filter());
                uint64_t Np = (uint64_t)(iter + 1) * (uint64_t) m_photons;
                for (int y = 0; y < film_size.y(); y++) {
                    for (int x = 0; x < film_size.x(); x++) {
                        const SPPMPixel &pixel = pixels[y * film_size.x() + x];
                        Spectrum value         = pixel.value / (iter + 1) +
                                         pixel.tau / (Np * math::Pi<Float> *
                                                      math::sqr(pixel.radius));
                        block->put(Vector2(x, y), value);
                    }
                }
                film->put(block);
                film->develop();
            }
        }
        Log(Info, "Rendering finished.");
    }

    APR_DECLARE_CLASS()
private:
    Float m_initial_radius  = 20.f;
    int m_iterations        = 1;
    int m_max_depth         = 5;
    int m_photons           = 100;
    int m_develop_frequency = 1 << 31;
};

APR_IMPLEMENT_CLASS(SPPMIntegrator, Integrator)
APR_INTERNAL_PLUGIN(SPPMIntegrator, "sppm")

} // namespace aspirin
