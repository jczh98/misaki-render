#include <misaki/interaction.h>
#include <misaki/phase.h>
#include <misaki/properties.h>
#include <misaki/warp.h>

namespace misaki {

class IsotropicPhaseFunction final : public PhaseFunction {
public:
    IsotropicPhaseFunction(const Properties &props) : PhaseFunction(props) {
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

APR_IMPLEMENT_CLASS(IsotropicPhaseFunction, PhaseFunction)
APR_INTERNAL_PLUGIN(IsotropicPhaseFunction, "isotropic")
} // namespace misaki