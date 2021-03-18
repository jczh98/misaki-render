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
#include <aspirin/utils.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class PathTracer final : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;
    using typename Base::Scene;
    using typename Base::Sensor;
    using Ray                = Ray<Float, Spectrum>;
    using RayDifferential    = RayDifferential<Float, Spectrum>;
    using ImageBlock         = ImageBlock<Float, Spectrum>;
    using Sampler            = Sampler<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    PathTracer(const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto total_spp = sensor->sampler()->sample_count();
        Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
            film_size.y(), total_spp);
        int m_block_size = APR_BLOCK_SIZE;
        BlockGenerator gen(film_size, m_block_size);
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
                    auto [offset, size] = gen.next_block();
                    block->set_offset(offset);
                    block->set_size(size);
                    render_block(scene, sensor, sampler, block, total_spp);
                    film->put(block);
                    pbar.update();
                }
            });
        pbar.done();
        Log(Info, "Rendering finished. (took {})",
            time_string(timer.value(), true));
        return true;
    }

    void render_block(const Scene *scene, const Sensor *sensor,
                      Sampler *sampler, ImageBlock *block,
                      size_t sample_count) const {
        block->clear();
        auto &size              = block->size();
        auto &offset            = block->offset();
        Float diff_scale_factor = Float(1) / std::sqrt(sample_count);
        for (int y = 0; y < size.y(); ++y) {
            for (int x = 0; x < size.x(); ++x) {
                Vector2 pos = Vector2(x, y);
                if (pos.x() >= size.x() || pos.y() >= size.y())
                    continue;
                pos = pos + offset.template cast<Float>();
                for (int s = 0; s < sample_count; ++s) {
                    render_sample(scene, sensor, sampler, block, pos,
                                  diff_scale_factor);
                }
            }
        }
    }

    void render_sample(const Scene *scene, const Sensor *sensor,
                       Sampler *sampler, ImageBlock *block, const Vector2 &pos,
                       Float diff_scale_factor) const {
        auto position_sample = pos + sampler->next2d();
        auto [ray, ray_weight] =
            sensor->sample_ray_differential(position_sample);
        ray.scale_differential(diff_scale_factor);
        auto result = sample(scene, sampler, ray);
        if (result)
            block->put(position_sample, *result);
    }

    std::optional<Color3> sample(const Scene *scene, Sampler *sampler,
                                 const RayDifferential &ray_) const {
        RayDifferential ray = ray_;
        Color3 throughput = Spectrum::Constant(1.f), result = Spectrum::Zero();
        Float emission_weight = 1.f, eta = 1.f;
        SurfaceInteraction si = scene->ray_intersect(ray);
        auto emitter          = si.emitter(scene);
        for (int depth = 1;; ++depth) {
            if (emitter != nullptr) {
                result += emitter->eval(si) * throughput * emission_weight;
            }
            if (depth >= m_rr_depth) {
                Float q = std::min(throughput.maxCoeff() * eta * eta, 0.95f);
                if (sampler->next1d() >= q)
                    break;
                throughput *= 1.f / q;
            }
            if ((uint32_t) depth >= (uint32_t) m_max_depth || !si.is_valid())
                break;
            // ------------------ Emitter Sampling --------------------------
            BSDFContext ctx;
            auto bsdf = si.bsdf(ray);
            if (has_flag(bsdf->flags(), BSDFFlags::Smooth)) {
                auto [ds, emitter_val] = scene->sample_emitter_direction(
                    si, sampler->next2d(), true);
                if (ds.pdf != 0.f) {
                    auto wo           = si.to_local(ds.d);
                    Spectrum bsdf_val = bsdf->eval(ctx, si, wo);
                    auto bsdf_pdf     = bsdf->pdf(ctx, si, wo);
                    Float mis = ds.delta ? 1.f : mis_weight(ds.pdf, bsdf_pdf);
                    result += throughput * bsdf_val * emitter_val * mis;
                }
            }
            // --------------------- BSDF Sampling ------------------------
            // Sample BSDF * cos(theta)
            auto [bs, bsdf_val] =
                bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());
            throughput = throughput * bsdf_val;
            eta *= bs.eta;
            ray                        = si.spawn_ray(si.to_world(bs.wo));
            SurfaceInteraction si_bsdf = scene->ray_intersect(ray);
            emitter                    = si_bsdf.emitter(scene);
            DirectionSample ds(si_bsdf, si);
            ds.object = emitter;
            if (emitter != nullptr) {
                auto emitter_pdf = !has_flag(bs.sampled_type, BSDFFlags::Delta)
                                       ? scene->pdf_emitter_direction(si, ds)
                                       : 0.f;
                emission_weight  = mis_weight(bs.pdf, emitter_pdf);
            }
            si = std::move(si_bsdf);
        }
        return result;
    }

    Float mis_weight(Float pdf_a, Float pdf_b) const {
        pdf_a *= pdf_a;
        pdf_b *= pdf_b;
        return pdf_a > 0.f ? pdf_a / (pdf_a + pdf_b) : 0.f;
    }

    APR_DECLARE_CLASS()
private:
    int m_max_depth = -1, m_rr_depth = 5;
    std::mutex m_mutex;
};

APR_IMPLEMENT_CLASS_VARIANT(PathTracer, Integrator)
APR_INTERNAL_PLUGIN(PathTracer, "path")

} // namespace aspirin