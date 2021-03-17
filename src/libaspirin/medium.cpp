#include <aspirin/medium.h>
#include <aspirin/phase.h>
#include <aspirin/properties.h>
#include <aspirin/scene.h>
#include <aspirin/volume.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Medium<Float, Spectrum>::Medium() : m_is_homogeneous(false) {}

template <typename Float, typename Spectrum>
Medium<Float, Spectrum>::Medium(const Properties &props) : m_id(props.id()) {

    for (auto &[name, obj] : props.objects()) {
        auto *phase = dynamic_cast<PhaseFunction *>(obj.get());
        if (phase) {
            if (m_phase_function)
                Throw(
                    "Only a single phase function can be specified per medium");
            m_phase_function = phase;
        }
    }
    if (!m_phase_function) {
        // Create a default isotropic phase function
        m_phase_function =
            PluginManager::instance()->create_object<PhaseFunction>(
                Properties("isotropic"));
    }
}

template <typename Float, typename Spectrum>
Medium<Float, Spectrum>::~Medium() {}

template <typename Float, typename Spectrum>
typename Medium<Float, Spectrum>::MediumInteraction
Medium<Float, Spectrum>::sample_interaction(const Ray &ray, Float sample,
                                            uint32_t channel) const {

    // initialize basic medium interaction fields
    MediumInteraction mi;
    mi.sh_frame = Frame3(ray.d);
    mi.wi       = -ray.d;

    auto [aabb_its, mint, maxt] = intersect_aabb(ray);
    if (std::isfinite(mint) || std::isfinite(maxt)) {
        mint = std::max(ray.mint, mint);
        maxt = std::min(ray.maxt, maxt);

        auto combined_extinction = get_combined_extinction(mi);
        Float m                  = combined_extinction[channel];
        Float sampled_t          = mint + (-std::log(1 - sample) / m);
        mi.p                     = ray(sampled_t);
        if (sampled_t <= maxt) {
            mi.t = sampled_t;
            std::tie(mi.sigma_s, mi.sigma_n, mi.sigma_t) =
                get_scattering_coefficients(mi);
        } else {
            mi.t = math::Infinity<Float>;
        }
        mi.combined_extinction = combined_extinction;
    } else {
        mint = 0.f;
        maxt = math::Infinity<Float>;
        mi.t = math::Infinity<Float>;
    }
    mi.medium = this;
    mi.mint   = mint;
    return mi;
}

template <typename Float, typename Spectrum>
std::pair<Spectrum, Spectrum>
Medium<Float, Spectrum>::eval_tr_and_pdf(const MediumInteraction &mi,
                                         const SurfaceInteraction &si) const {

    Float t      = std::min(mi.t, si.t) - mi.mint;
    Spectrum tr  = (-t * mi.combined_extinction).array().exp();
    Spectrum pdf = si.t < mi.t ? tr : tr * mi.combined_extinction;
    return { tr, pdf };
}

APR_IMPLEMENT_CLASS_VARIANT(Medium, Object, "medium")
APR_INSTANTIATE_CLASS(Medium)

} // namespace aspirin