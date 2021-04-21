#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/film.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/medium.h>
#include <aspirin/mesh.h>
#include <aspirin/phase.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <aspirin/utils.h>
#include <fstream>
#include <tbb/parallel_for.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class VolumetricPathTracer final : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;
    using typename Base::RadianceQuery;
    using typename Base::Scene;
    using typename Base::Sensor;
    using Ray                  = Ray<Float, Spectrum>;
    using RayDifferential      = RayDifferential<Float, Spectrum>;
    using ImageBlock           = ImageBlock<Float, Spectrum>;
    using Sampler              = Sampler<Float, Spectrum>;
    using Medium               = Medium<Float, Spectrum>;
    using MediumInteraction    = MediumInteraction<Float, Spectrum>;
    using Interaction          = Interaction<Float, Spectrum>;
    using SurfaceInteraction   = SurfaceInteraction<Float, Spectrum>;
    using PhaseFunction        = PhaseFunction<Float, Spectrum>;
    using PhaseFunctionContext = PhaseFunctionContext<Float, Spectrum>;
    using DirectionSample      = DirectionSample<Float, Spectrum>;
    using MediumPtr            = const Medium *;

    VolumetricPathTracer(const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) {
        auto film      = sensor->film();
        auto film_size = film->size();
        auto total_spp = sensor->sampler()->sample_count();
        Log(Info, "Starting render job ({}x{}, {} sample)", film_size.x(),
            film_size.y(), total_spp);
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
            sensor->sample_ray_differential(position_sample, sampler->next2d());
        ray.scale_differential(diff_scale_factor);
        auto result = sample(scene, sampler, ray, sensor->medium());
        block->put(position_sample, result);
    }

    Spectrum sample(const Scene *scene, Sampler *sampler,
                    const RayDifferential &ray_,
                    const Medium *initial_medium = nullptr) const {
        RayDifferential ray = ray_;
        Spectrum throughput = Spectrum::Constant(1.f),
                 result     = Spectrum::Zero();
        Float eta           = 1.f;
        MediumPtr medium    = initial_medium;
        bool scattered = false, null_chain = true;
        RadianceQuery rtype   = RadianceQuery::Radiance;
        SurfaceInteraction si = scene->ray_intersect(ray);
        uint32_t channel = std::min<uint32_t>(sampler->next1d() * 3, 3 - 1);
        for (int depth = 1; depth <= m_max_depth || m_max_depth < 0; depth++) {
            MediumInteraction mi;
            Float medium_pdf;
            if (medium != nullptr) {
                Ray medium_ray           = ray;
                medium_ray.mint          = 0;
                medium_ray.maxt          = si.t;
                std::tie(mi, medium_pdf) = medium->sample_interaction(
                    medium_ray, sampler->next1d(), channel);
            }
            if (medium != nullptr && mi.is_valid()) {
                throughput *= mi.sigma_s * mi.transmittance / medium_pdf;
                const PhaseFunction *phase = medium->phase_function();
                PhaseFunctionContext phase_ctx(sampler);
                /*
                 * Direct illumination sampling
                 */
                if (rtype & RadianceQuery::DirectMediumRadiance) {
                    auto [ds, spec] = sample_attenuated_emitter(
                        scene, medium, si, sampler->next2d());
                    if (!is_black(spec)) {
                        result += throughput * spec *
                                  phase->eval(phase_ctx, mi, ds.d);
                    }
                }

                if ((depth >= m_max_depth && m_max_depth > 0) ||
                    !(rtype & RadianceQuery::IndirectMediumRadiance))
                    break;
                /**
                 * Phase function sampling
                 */
                auto [phase_wo, phase_pdf, phase_val] =
                    phase->sample(phase_ctx, mi, sampler->next2d());
                if (phase_val == 0.f)
                    break;
                throughput *= phase_val;
                // Trace a ray in phase sampled direction
                ray        = mi.spawn_ray(phase_wo);
                ray.mint   = 0;
                si         = scene->ray_intersect(ray);
                null_chain = false;
                scattered  = true;
            } else {
                if (medium != nullptr) {
                    throughput *= mi.transmittance / medium_pdf;
                }
                /*
                 * Sample surface integral
                 */
                if (!si.is_valid()) {
                    if ((rtype & RadianceQuery::EmittedRadiance) &&
                        (!m_hide_emitter || scattered)) {
                        Spectrum value =
                            throughput * ((scene->environment() != nullptr)
                                              ? scene->environment()->eval(si)
                                              : Spectrum::Zero());
                        if (medium != nullptr)
                            value *= medium->eval_transmittance(ray);
                        result += value;
                    }
                    break;
                }
                // Compute emitted radiance
                if (si.shape->emitter() != nullptr &&
                    (rtype & RadianceQuery::EmittedRadiance) &&
                    (!m_hide_emitter || scattered)) {
                    result += throughput * si.shape->emitter()->eval(si);
                }
                /*
                 * Direct illumination sampling
                 */
                BSDFContext ctx;
                auto bsdf = si.bsdf(ray);
                if ((rtype & RadianceQuery::DirectSurfaceRadiance) &&
                    has_flag(bsdf->flags(), BSDFFlags::Smooth)) {
                    auto [ds, emitter_val] = sample_attenuated_emitter(
                        scene, medium, si, sampler->next2d());
                    if (ds.pdf != 0.f) {
                        auto wo           = si.to_local(ds.d);
                        Spectrum bsdf_val = bsdf->eval(ctx, si, wo);
                        Float bsdf_pdf    = bsdf->pdf(ctx, si, wo);
                        Float weight      = mis_weight(ds.pdf, bsdf_pdf);
                        result += throughput * emitter_val * bsdf_val;
                    }
                }
                /*
                 * BSDF Sampling
                 */
                auto [bs, bsdf_val] =
                    bsdf->sample(ctx, si, sampler->next1d(), sampler->next2d());

                if (is_black(bsdf_val))
                    break;

                int recursive_type = 0;
                if ((depth + 1 < m_max_depth || m_max_depth < 0) &&
                    (rtype & RadianceQuery::IndirectSurfaceRadiance))
                    recursive_type |= RadianceQuery::RadianceNoEmission;

                // Recursively gather direct illumination
                if ((depth < m_max_depth || m_max_depth < 0) &&
                    (rtype & RadianceQuery::DirectSurfaceRadiance) &&
                    has_flag(bs.sampled_type, BSDFFlags::Delta) &&
                    (!has_flag(bs.sampled_type, BSDFFlags::Null) ||
                     null_chain)) {
                    recursive_type |= RadianceQuery::EmittedRadiance;
                    null_chain = true;
                } else {
                    null_chain &= has_flag(bs.sampled_type, BSDFFlags::Null);
                }
                // Potentially stop the recursion if there is nothing more to do
                if (recursive_type == 0)
                    break;
                rtype = (RadianceQuery) recursive_type;

                const auto wo = si.to_world(bs.wo);

                throughput *= bsdf_val;
                eta *= bs.eta;
                if (si.is_medium_transition())
                    medium = si.target_medium(wo);

                ray = si.spawn_ray(wo);

                si = scene->ray_intersect(ray);

                scattered |= !has_flag(bs.sampled_type, BSDFFlags::Null);
            }

            // Russian roulette
            if (depth + 1 >= m_rr_depth) {
                Float q =
                    std::min(throughput.maxCoeff() * eta * eta, Float(0.95));
                if (sampler->next1d() >= q)
                    break;
                throughput /= q;
            }
        }
        return result;
    }

    Spectrum evaluate_transmittance(const Scene *scene, const Vector3 &ref,
                                    const Vector3 &p,
                                    const Medium *medium) const {
        Vector3 d       = p - ref;
        Float remaining = d.norm();
        d /= remaining;
        Ray ray(ref, d, math::RayEpsilon<Float>,
                remaining * (1 - math::ShadowEpsilon<Float>), 0);
        Spectrum transmittance = Spectrum::Constant(1);
        while (remaining) {
            SurfaceInteraction si = scene->ray_intersect(ray);
            if (si.is_valid() &&
                !has_flag(si.bsdf()->flags(), BSDFFlags::Null)) {
                return Spectrum::Zero();
            }
            if (medium) {
                Ray medium_ray  = ray;
                medium_ray.mint = 0.f;
                medium_ray.maxt = std::min(si.t, remaining);
                transmittance *= medium->eval_transmittance(medium_ray);
            }
            if (!si.is_valid() || is_black(transmittance))
                break;
            BSDFContext ctx;
            const auto bsdf = si.bsdf();
            ctx.type_mask   = +BSDFFlags::Null;
            const auto wo   = si.to_local(ray.d);
            si.wi           = -wo;
            transmittance *= bsdf->eval(ctx, si, wo);
            if (si.is_medium_transition()) {
                if (medium != si.target_medium(-d))
                    return Spectrum::Zero();
                medium = si.target_medium(d);
            }
            ray.o = ray(si.t);
            remaining -= si.t;
            ray.maxt = remaining * (1 - math::ShadowEpsilon<Float>);
            ray.mint = math::RayEpsilon<Float>;
        }
        return transmittance;
    }

    std::pair<DirectionSample, Spectrum>
    sample_attenuated_emitter(const Scene *scene, const Medium *medium,
                              const SurfaceInteraction &ref,
                              const Vector2 &sample_) const {
        DirectionSample ds;
        Spectrum spec;
        Vector2 sample(sample_);
        Float emitter_pdf = 1.f;
        // Uniform pick an emitter
        if (!scene->emitters().empty()) {
            if (scene->emitters().size() == 1) {
                std::tie(ds, spec) =
                    scene->emitters()[0]->sample_direction(ref, sample);
            } else {
                emitter_pdf /= scene->emitters().size();
                auto index = std::min(
                    uint32_t(sample.x() * (Float) scene->emitters().size()),
                    (uint32_t) scene->emitters().size() - 1);
                sample.x() = (sample.x() - index * emitter_pdf) *
                             scene->emitters().size();
                std::tie(ds, spec) =
                    scene->emitters()[index]->sample_direction(ref, sample);
            }
        }
        if (ds.pdf != 0.f) {
            if (ref.shape != nullptr && ref.is_medium_transition())
                medium = ref.target_medium(ds.d);
            spec *= evaluate_transmittance(scene, ref.p, ds.p, medium) /
                    emitter_pdf;
            ds.pdf *= emitter_pdf;
            return { ds, spec };
        } else {
            return { ds, Spectrum::Zero() };
        }
    }

    Float mis_weight(Float pdf_a, Float pdf_b) const {
        pdf_a *= pdf_a;
        pdf_b *= pdf_b;
        return pdf_a > 0.f ? pdf_a / (pdf_a + pdf_b) : 0.f;
    }

    APR_DECLARE_CLASS()
private:
    int m_max_depth = -1, m_rr_depth = 5;
    bool m_hide_emitter = false;
    std::mutex m_mutex;
};

APR_IMPLEMENT_CLASS_VARIANT(VolumetricPathTracer, Integrator)
APR_INTERNAL_PLUGIN(VolumetricPathTracer, "volpath")

} // namespace aspirin