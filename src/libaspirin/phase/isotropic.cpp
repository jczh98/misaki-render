#include <aspirin/interaction.h>
#include <aspirin/phase.h>
#include <aspirin/properties.h>
#include <aspirin/warp.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class IsotropicPhaseFunction final : public PhaseFunction<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float);
    using Base = PhaseFunction<Float, Spectrum>;
    using Base::m_flags;
    using typename Base::MediumInteraction;
    using typename Base::PhaseFunctionContext;

    IsotropicPhaseFunction(const Properties &props) : Base(props) {
        m_flags = +PhaseFunctionFlags::Isotropic;
    }

    std::tuple<Vector3, Float, Float>
    sample(const PhaseFunctionContext &, const MediumInteraction &,
           const Vector2 &sample) const override {
        auto wo  = warp::square_to_uniform_sphere(sample);
        auto pdf = warp::square_to_uniform_sphere_pdf(wo);
        return { wo, pdf, 1.f };
    }

    Float eval(const PhaseFunctionContext &, const MediumInteraction &,
               const Vector3 &wo) const override {
        return warp::square_to_uniform_sphere_pdf(wo);
    }

    std::string to_string() const override {
        return "IsotropicPhaseFunction[]";
    }

    APR_DECLARE_CLASS()
private:
};

APR_IMPLEMENT_CLASS_VARIANT(IsotropicPhaseFunction, PhaseFunction)
APR_INTERNAL_PLUGIN(IsotropicPhaseFunction, "isotropic")
} // namespace aspirin