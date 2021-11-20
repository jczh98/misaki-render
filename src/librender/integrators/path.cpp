#include <misaki/render/bsdf.h>
#include <misaki/render/emitter.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/core/logger.h>
#include <misaki/render/mesh.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
#include <misaki/render/records.h>
#include <misaki/render/scene.h>
#include <misaki/render/sensor.h>
#include <misaki/core/utils.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace misaki {

class PathTracer final : public Integrator {
public:
    PathTracer(const Properties &props) : Integrator(props) {}

    bool render(Scene *scene, Sensor *sensor) {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto total_spp = sensor->sampler()->sample_count();
        Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
            film_size.y(), total_spp);
        int m_block_size = APR_BLOCK_SIZE;
        BlockGenerator gen(film_size, Eigen::Vector2i::Zero(), m_block_size);
        size_t total_blocks = gen.block_count();
        ProgressBar pbar(total_blocks, 70);
        Timer timer;
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, total_blocks, 1),
            [&](const tbb::blocked_range<size_t> &range) {
                auto sampler          = sensor->sampler()->clone();
                ref<ImageBlock> block = new ImageBlock(
                    Eigen::Vector2i::Constant(m_block_size), film->filter());
                for (auto i = range.begin(); i != range.end(); ++i) {
                    auto [offset, size, block_id] = gen.next_block();
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
        float diff_scale_factor = float(1) / std::sqrt(sample_count);
        for (int y = 0; y < size.y(); ++y) {
            for (int x = 0; x < size.x(); ++x) {
                Eigen::Vector2f pos = Eigen::Vector2f(x, y);
                if (pos.x() >= size.x() || pos.y() >= size.y())
                    continue;
                pos = pos + offset.template cast<float>();
                for (int s = 0; s < sample_count; ++s) {
                    render_sample(scene, sensor, sampler, block, pos,
                                  diff_scale_factor);
                }
            }
        }
    }

    void render_sample(const Scene *scene, const Sensor *sensor,
                       Sampler *sampler, ImageBlock *block, const Eigen::Vector2f &pos,
                       float diff_scale_factor) const {
        auto position_sample = pos + sampler->next2d();
        auto [ray, ray_weight] =
            sensor->sample_ray_differential(position_sample, sampler->next2d());
        ray.scale_differential(diff_scale_factor);
        auto result = sample(scene, sampler, ray);
        block->put(position_sample, result);
    }

    Spectrum sample(const Scene *scene, Sampler *sampler,
                    const RayDifferential &ray_) const {
        RadianceQuery type    = RadianceQuery::Radiance;
        RayDifferential ray   = ray_;
        Spectrum throughput   = Spectrum::Constant(1.f),
                 result       = Spectrum::Zero();
        float eta             = 1.f;
        bool scattered        = false;
        SceneInteraction si = scene->ray_intersect(ray);
        for (int depth = 1; depth <= m_max_depth || m_max_depth < 0; depth++) {
            if (!si.is_valid()) {
                // If no intersection, compute the environment illumination
                if ((type & RadianceQuery::EmittedRadiance) &&
                    (!m_hide_emitter || scattered)) {
                    if (scene->environment() != nullptr)
                        result += throughput * scene->environment()->eval(si);
                }
                break;
            }
            auto emitter = si.shape->emitter();
            // Compute emitted radiance
            if (emitter != nullptr && (type & RadianceQuery::EmittedRadiance) &&
                (!m_hide_emitter || scattered)) {
                result += throughput * emitter->eval(si);
            }
            if (depth >= m_max_depth && m_max_depth > 0)
                break;
            /*
             * Direct illumination sampling
             */
            BSDFContext ctx;
            DirectIllumSample ds;
            auto bsdf = si.bsdf(ray);
            if ((type & RadianceQuery::DirectSurfaceRadiance) &&
                has_flag(bsdf->flags(), BSDFFlags::Smooth)) {
                Spectrum emitter_val;
                std::tie(ds, emitter_val) = scene->sample_emitter_direct(
                    si, sampler->next2d(), true);
                if (ds.pdf != 0.f) {
                    auto wo           = si.to_local(ds.d);
                    Spectrum bsdf_val = bsdf->eval(ctx, si, wo);
                    float bsdf_pdf    = bsdf->pdf(ctx, si, wo);
                    float weight      = mis_weight(ds.pdf, bsdf_pdf);
                    result += throughput * emitter_val * bsdf_val * weight;
                }
            }
            /*
             * BSDF Sampling
             */
            auto [bs, bsdf_val] =
                bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());
            scattered |= bs.sampled_type != (uint32_t) BSDFFlags::Null;

            const auto wo    = si.to_world(bs.wo);
            bool hit_emitter = false;
            Spectrum value   = Spectrum::Zero();

            ray                        = si.spawn_ray(wo);
            SceneInteraction si_bsdf = scene->ray_intersect(ray);

            if (si_bsdf.is_valid()) {
                emitter = si_bsdf.shape->emitter();
                if (emitter != nullptr) {
                    value       = emitter->eval(si_bsdf);
                    ds.set_query(ray, si_bsdf);
                    hit_emitter = true;
                }
            } else {
                // Intersected nothing or environment
                if (scene->environment() != nullptr) {
                    if (m_hide_emitter && !scattered)
                        break;
                    value       = scene->environment()->eval(si);
                    hit_emitter = true;
                } else
                    break;
            }
            throughput *= bsdf_val;
            eta *= bs.eta;

            // If an emitter was hit, estimate the illumination
            if (hit_emitter && (type & RadianceQuery::DirectSurfaceRadiance)) {
                auto emitter_pdf = !has_flag(bs.sampled_type, BSDFFlags::Delta)
                                       ? scene->pdf_emitter_direct(ds)
                                       : 0.f;
                result += throughput * value * mis_weight(bs.pdf, emitter_pdf);
            }
            // Indirect illumination
            if (!si.is_valid() ||
                !(type & RadianceQuery::IndirectSurfaceRadiance))
                break;

            type = RadianceQuery::RadianceNoEmission;

            si = std::move(si_bsdf);

            // Russian roulette
            if (depth + 1 >= m_rr_depth) {
                float q =
                    std::min(throughput.maxCoeff() * eta * eta, float(0.95));
                if (sampler->next1d() >= q)
                    break;
                throughput /= q;
            }
        }
        return result;
    }

    float mis_weight(float pdf_a, float pdf_b) const {
        pdf_a *= pdf_a;
        pdf_b *= pdf_b;
        return pdf_a > 0.f ? pdf_a / (pdf_a + pdf_b) : 0.f;
    }

    MSK_DECLARE_CLASS()
private:
    int m_max_depth = -1, m_rr_depth = 5;
    bool m_hide_emitter = false;
    std::mutex m_mutex;
};

MSK_IMPLEMENT_CLASS(PathTracer, Integrator)
MSK_REGISTER_INSTANCE(PathTracer, "path")

} // namespace misaki