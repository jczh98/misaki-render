#include <fstream>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/core/utils.h>
#include <misaki/render/bsdf.h>
#include <misaki/render/emitter.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/render/medium.h>
#include <misaki/render/mesh.h>
#include <misaki/render/phase.h>
#include <misaki/render/records.h>
#include <misaki/render/scene.h>
#include <misaki/render/sensor.h>
#include <tbb/parallel_for.h>

namespace misaki {

class VolumetricPathTracer final : public MonteCarloIntegrator {
public:
    VolumetricPathTracer(const Properties &props)
        : MonteCarloIntegrator(props) {}

    virtual Spectrum sample(const Scene *scene, Sampler *sampler,
                            const RayDifferential &ray_,
                            const Medium *initial_medium) const override {
        RayDifferential ray  = ray_;
        Spectrum throughput  = Spectrum::Constant(1.f),
                 result      = Spectrum::Zero();
        float eta            = 1.f;
        const Medium *medium = initial_medium;
        bool ms_flag, scattered = false, null_chain = true,
                      emitted_radiance = true;
        SceneInteraction si            = scene->ray_intersect(ray);
        MediumSample ms;
        uint32_t channel = std::min<uint32_t>(sampler->next1d() * 3, 3 - 1);
        for (int depth = 1; depth <= m_max_depth || m_max_depth < 0; depth++) {
            if (medium)
                std::tie(ms_flag, ms) = medium->sample_distance(
                    ray::spawn(ray, 0, si.t), sampler->next1d(), channel);
            if (medium && ms_flag) {
                throughput *= ms.sigma_s * ms.transmittance / ms.pdf;
                const PhaseFunction *phase = medium->phase_function();
                PhaseFunctionContext phase_ctx(sampler);

                auto [ds, spec] = scene->sample_attenuated_emitter_direct(
                    si, medium, sampler->next2d());
                if (!is_black(spec)) {
                    result +=
                        throughput * spec * phase->eval(phase_ctx, ms, ds.d);
                }

                if ((depth + 1 >= m_max_depth && m_max_depth > 0))
                    break;

                /**
                 * Phase function sampling
                 */
                auto [phase_wo, phase_pdf, phase_val] =
                    phase->sample(phase_ctx, ms, sampler->next2d());
                if (phase_val == 0.f)
                    break;
                throughput *= phase_val;
                // Trace a ray in phase sampled direction
                ray        = ray::spawn<false>(ms.p, phase_wo);
                si         = scene->ray_intersect(ray);
                null_chain = false;
                scattered  = true;
            } else {
                if (medium) {
                    throughput *= ms.transmittance / ms.pdf;
                }
                /*
                 * Sample surface integral
                 */
                if (!si.is_valid()) {
                    if (emitted_radiance && (!m_hide_emitter || scattered)) {
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
                if (si.shape->emitter() != nullptr && emitted_radiance &&
                    (!m_hide_emitter || scattered)) {
                    result += throughput * si.shape->emitter()->eval(si);
                }
                /*
                 * Direct illumination sampling
                 */
                BSDFContext ctx;
                auto bsdf = si.bsdf(ray);
                if (has_flag(bsdf->flags(), BSDFFlags::Smooth)) {
                    auto [ds, emitter_val] =
                        scene->sample_attenuated_emitter_direct(
                            si, medium, sampler->next2d());
                    if (ds.pdf != 0.f) {
                        auto wo           = si.to_local(ds.d);
                        Spectrum bsdf_val = bsdf->eval(ctx, si, wo);
                        float bsdf_pdf    = bsdf->pdf(ctx, si, wo);
                        float weight      = mis_weight(ds.pdf, bsdf_pdf);
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

                emitted_radiance = false;
                // Recursively gather indirect illumination
                bool recursive = false;
                if (depth + 1 < m_max_depth || m_max_depth < 0)
                    recursive = true;

                // Recursively gather direct illumination
                if ((depth < m_max_depth || m_max_depth < 0) &&
                    has_flag(bs.sampled_type, BSDFFlags::Delta) &&
                    (!has_flag(bs.sampled_type, BSDFFlags::Null) ||
                     null_chain)) {
                    emitted_radiance = true;
                    recursive        = true;
                    null_chain       = true;
                } else {
                    null_chain &= has_flag(bs.sampled_type, BSDFFlags::Null);
                }

                if (!recursive)
                    break;

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

MSK_IMPLEMENT_CLASS(VolumetricPathTracer, MonteCarloIntegrator)
MSK_REGISTER_INSTANCE(VolumetricPathTracer, "volpath")

} // namespace misaki