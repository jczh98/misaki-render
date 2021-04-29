#include "nanoflann.hpp"
#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/film.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/sampler.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <aspirin/utils.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace aspirin {

class PhotonMapper : public Integrator {
public:
    struct Photon {
        Photon(Vector3 p, Spectrum power, Vector3 wi)
            : p(std::move(p)), power(std::move(power)), wi(std::move(wi)) {}
        Vector3 p;
        Spectrum power;
        Vector3 wi;
    };

    struct PhotonMap {
        std::vector<Photon> photons;

        inline size_t kdtree_get_point_count() const { return photons.size(); }
        inline float kdtree_distance(const float *p1, const size_t idx_p2,
                                     size_t) const {
            const float d0 = p1[0] - photons[idx_p2].p.x();
            const float d1 = p1[1] - photons[idx_p2].p.y();
            const float d2 = p1[2] - photons[idx_p2].p.z();
            return d0 * d0 + d1 * d1 + d2 * d2;
        }

        inline float kdtree_get_pt(const size_t idx, int dim) const {
            if (dim == 0)
                return photons[idx].p.x();
            else if (dim == 1)
                return photons[idx].p.y();
            else
                return photons[idx].p.z();
        }

        inline Photon &kdtree_get_data(const size_t idx) {
            return photons[idx];
        }

        template <class BBOX> bool kdtree_get_bbox(BBOX & /*bb*/) const {
            return false;
        }
    };
    using PhotonKDTree = nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<float, PhotonMap>, PhotonMap, 3>;

    PhotonMapper(const Properties &props)
        : Integrator(props),
          m_global_photon_kdtree(3, m_global_photon_map,
                                 nanoflann::KDTreeSingleIndexAdaptorParams()) {
        m_photon_count = props.int_("photon_count", 10000);
        m_radius       = props.float_("photon_radius", 35.f);
    }

    bool render(Scene *scene, Sensor *sensor) override {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto sampler_  = PluginManager::instance()->create_object<Sampler>(
            Properties("independent"));
        Log(Info, "Gathering photons from emitters...");
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, m_photon_count, 1),
            [&](const tbb::blocked_range<size_t> &range) {
                ref<Sampler> sampler = sampler_->clone();
                for (auto i = range.begin(); i != range.end(); i++) {
                    sampler->seed(i);
                    auto emitter_sel_pdf = 1.f / scene->emitters().size();
                    auto emitter_sample  = sampler->next1d();
                    auto index =
                        std::min(uint32_t(emitter_sample *
                                          (Float) scene->emitters().size()),
                                 (uint32_t) scene->emitters().size() - 1);
                    const auto emitter = scene->emitters()[index];
                    auto [ray, flux]   = emitter->sample_ray(sampler->next2d(),
                                                           sampler->next2d());
                    flux /= emitter_sel_pdf;
                    trace_photon(scene, sampler, ray, flux);
                }
            });
        // Build KDTree
        m_global_photon_kdtree.buildIndex();
        Log(Info, "Gathered {} photons and built KDTree.", m_photon_count);
        // Render from camera path
        int m_block_size = APR_BLOCK_SIZE;
        BlockGenerator gen(film_size, Vector2i::Zero(), m_block_size);
        size_t total_blocks = gen.block_count();
        ProgressBar pbar(total_blocks, 70);
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
                    render_block(scene, sensor, sampler, block);
                    film->put(block);
                    pbar.update();
                }
            });
        pbar.done();
        Log(Info, "Rendering finished. (took {})",
            time_string(timer.value(), true));
    }

    void trace_photon(Scene *scene, Sampler *sampler, const Ray &ray_,
                      const Spectrum &flux) {
        Spectrum throughput   = flux;
        Ray ray               = ray_;
        SurfaceInteraction si = scene->ray_intersect(ray);
        for (int depth = 0; depth < m_max_depth; depth++) {
            if (!si.is_valid())
                break;
            auto bsdf = si.bsdf(ray);
            if (has_flag(bsdf->flags(), BSDFFlags::Diffuse)) {
                tbb::spin_mutex::scoped_lock lock(m_mutex);
                Photon photon(si.p, throughput, -ray.d);
                m_global_photon_map.photons.push_back(photon);
            }
            BSDFContext ctx(TransportMode::Importance);
            auto [bs, bsdf_val] =
                bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());
            if (bs.pdf == 0.f || is_black(bsdf_val))
                break;
            throughput *= bsdf_val;
            // Russian roulette
            Float q = std::min(throughput.maxCoeff(), Float(0.95));
            if (sampler->next1d() >= q)
                break;
            throughput /= q;
            ray = si.spawn_ray(si.to_world(bs.wo));
            si  = scene->ray_intersect(ray);
        }
    }

    void render_block(const Scene *scene, const Sensor *sensor,
                      Sampler *sampler, ImageBlock *block) const {
        block->clear();
        auto &size   = block->size();
        auto &offset = block->offset();
        for (int y = 0; y < size.y(); ++y) {
            for (int x = 0; x < size.x(); ++x) {
                Vector2 pos = Vector2(x, y);
                if (pos.x() >= size.x() || pos.y() >= size.y())
                    continue;
                pos = pos + offset.template cast<Float>();
                render_pixel(scene, sensor, sampler, block, pos);
            }
        }
    }

    void render_pixel(const Scene *scene, const Sensor *sensor,
                      Sampler *sampler, ImageBlock *block,
                      const Vector2 &pos) const {
        auto position_sample = pos + sampler->next2d();
        auto [ray, ray_weight] =
            sensor->sample_ray_differential(position_sample, sampler->next2d());
        auto result = estimate(scene, sampler, ray);
        block->put(position_sample, result);
    }

    Spectrum estimate(const Scene *scene, Sampler *sampler,
                      const RayDifferential &ray_) const {
        Spectrum result     = Spectrum ::Zero(),
                 throughput = Spectrum ::Constant(1);
        Ray ray             = ray_;
        auto si             = scene->ray_intersect(ray);
        for (int depth = 0; depth < m_max_depth; depth++) {
            if (!si.is_valid()) {
                break;
            }
            auto emitter = si.shape->emitter();
            if (emitter != nullptr && depth == 0) {
                result += throughput * emitter->eval(si);
            }
            if (depth >= m_max_depth && m_max_depth > 0)
                break;
            BSDFContext ctx;
            auto bsdf = si.bsdf(ray);
            if (has_flag(bsdf->flags(), BSDFFlags::Diffuse)) {
                result += throughput * photon_density_estimation(si, ctx, ray);
                break;
            }
            auto [bs, bsdf_val] =
                bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());
            throughput *= bsdf_val;
            // Russian roulette
            Float q = std::min(throughput.maxCoeff(), Float(0.95));
            if (sampler->next1d() >= q)
                break;
            throughput /= q;
            ray = si.spawn_ray(si.to_world(bs.wo));
            si  = std::move(scene->ray_intersect(ray));
        }
        return result;
    }

    Spectrum photon_density_estimation(const SurfaceInteraction &si_,
                                       const BSDFContext &ctx,
                                       const Ray &ray) const {
        SurfaceInteraction si(si_);
        Spectrum tau = Spectrum ::Zero();
        std::vector<std::pair<size_t, float>> matches;
        const float query_pt[3] = { si.p.x(), si.p.y(), si.p.z() };
        auto search_result      = m_global_photon_kdtree.radiusSearch(
            query_pt, m_radius * m_radius, matches, nanoflann::SearchParams());
        for (int i = 0; i < search_result; i++) {
            size_t idx    = matches[i].first;
            Photon photon = m_global_photon_map.photons[idx];
            auto bsdf     = si.bsdf(ray);
            const auto wo = si.to_local(photon.wi);
            tau +=
                bsdf->eval(ctx, si, wo) / Frame3::cos_theta(wo) * photon.power;
        }
        return tau * math::InvPi<Float> /
               (math::sqr(m_radius) * m_photon_count);
    }

    APR_DECLARE_CLASS()
private:
    PhotonKDTree m_global_photon_kdtree;
    PhotonMap m_global_photon_map;
    int m_photon_count;
    int m_max_depth = 5;
    float m_radius;
    tbb::spin_mutex m_mutex;
};

APR_IMPLEMENT_CLASS(PhotonMapper, Integrator)
APR_INTERNAL_PLUGIN(PhotonMapper, "photonmapper")

} // namespace aspirin