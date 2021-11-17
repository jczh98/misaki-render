#include <misaki/render/interaction.h>
#include <misaki/render/phase.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
#include <misaki/core/warp.h>

namespace misaki {

class IsotropicPhaseFunction final : public PhaseFunction {
public:
    IsotropicPhaseFunction(const Properties &props) : PhaseFunction(props) {
        m_flags = +PhaseFunctionFlags::Isotropic;
    }

    std::tuple<Eigen::Vector3f, float, float>
    sample(const PhaseFunctionContext &, const MediumInteraction &,
           const Eigen::Vector2f &sample) const override {
        auto wo  = warp::square_to_uniform_sphere(sample);
        auto pdf = warp::square_to_uniform_sphere_pdf(wo);
        return { wo, pdf, 1.f };
    }

    float eval(const PhaseFunctionContext &, const MediumInteraction &,
               const Eigen::Vector3f &wo) const override {
        return warp::square_to_uniform_sphere_pdf(wo);
    }

    std::string to_string() const override {
        return "IsotropicPhaseFunction[]";
    }

    MSK_DECLARE_CLASS()
private:
};

MSK_IMPLEMENT_CLASS(IsotropicPhaseFunction, PhaseFunction)
MSK_REGISTER_INSTANCE(IsotropicPhaseFunction, "isotropic")
} // namespace misaki