#pragma once

#include "fwd.h"
#include "object.h"
#include "ray.h"

namespace aspirin {

template <typename Float, typename Spectrum> class APR_EXPORT Medium : Object {

public:
    APR_IMPORT_CORE_TYPES(Float)
    using Ray                = Ray<Float, Spectrum>;
    using PhaseFunction      = PhaseFunction<Float, Spectrum>;
    using Sampler            = Sampler<Float, Spectrum>;
    using Scene              = Scene<Float, Spectrum>;
    using Texture            = Texture<Float, Spectrum>;
    using MediumInteraction  = MediumInteraction<Float, Spectrum>;
    using SurfaceInteraction = SurfaceInteraction<Float, Spectrum>;

    virtual std::tuple<bool, Float, Float>
    intersect_aabb(const Ray &ray) const = 0;

    /// Returns the medium's majorant used for delta tracking
    virtual Spectrum
    get_combined_extinction(const MediumInteraction &mi) const = 0;

    /// Returns the medium coefficients Sigma_s, Sigma_n and Sigma_t evaluated
    /// at a given MediumInteraction mi
    virtual std::tuple<Spectrum, Spectrum, Spectrum>
    get_scattering_coefficients(const MediumInteraction &mi) const = 0;

    /// Sample a free-flight distance in the medium.
    MediumInteraction sample_interaction(const Ray &ray, Float sample,
                                         uint32_t channel) const;

    /// Compute the transmittance and PDF
    std::pair<Spectrum, Spectrum>
    eval_tr_and_pdf(const MediumInteraction &mi,
                    const SurfaceInteraction &si) const;

    const PhaseFunction *phase_function() const {
        return m_phase_function.get();
    }

    bool is_homogeneous() const { return m_is_homogeneous; }
    std::string id() const override { return m_id; }
    std::string to_string() const override = 0;

    APR_DECLARE_CLASS()
protected:
    Medium();
    Medium(const Properties &props);
    virtual ~Medium();

protected:
    ref<PhaseFunction> m_phase_function;
    bool m_is_homogeneous;

    std::string m_id;
};
APR_EXTERN_CLASS(Medium)

} // namespace aspirin