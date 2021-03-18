#include <aspirin/interaction.h>
#include <aspirin/medium.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>
#include <aspirin/volume.h>
#include <iostream>
namespace aspirin {

template <typename Float, typename Spectrum>
class HomogeneousMedium final : public Medium<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Medium<Float, Spectrum>;
    using Base::m_is_homogeneous;
    using typename Base::MediumInteraction;
    using typename Base::Ray;
    using typename Base::Sampler;
    using typename Base::Scene;
    using typename Base::SurfaceInteraction;
    using typename Base::Texture;
    using Volume      = Volume<Float, Spectrum>;
    using Interaction = Interaction<Float, Spectrum>;

    HomogeneousMedium(const Properties &props) : Base(props) {
        m_is_homogeneous = true;
        m_albedo         = props.volume<Volume>("albedo", 0.75f);
        m_sigmat         = props.volume<Volume>("sigma_t", 1.f);
        m_scale = props.get_float("scale", 1.0f);
    }

    APR_INLINE auto eval_sigmat(const MediumInteraction &mi) const {
        return m_sigmat->eval(mi) * m_scale;
    }

    Spectrum
    get_combined_extinction(const MediumInteraction &mi) const override {
        return eval_sigmat(mi);
    }

    std::tuple<Spectrum, Spectrum, Spectrum>
    get_scattering_coefficients(const MediumInteraction &mi) const override {
        Spectrum sigmat = eval_sigmat(mi);
        Spectrum sigmas = sigmat * m_albedo->eval(mi);
        Spectrum sigman = Spectrum::Zero();
        return { sigmas, sigman, sigmat };
    }

    std::tuple<bool, Float, Float> intersect_aabb(const Ray &) const override {
        return { true, 0.f, math::Infinity<Float> };
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "HomogeneousMedium[" << std::endl
            << "  albedo  = " << string::indent(m_albedo->to_string())
            << std::endl
            << "  sigma_t = " << string::indent(m_sigmat->to_string())
            << std::endl
            << "  scale   = " << m_scale << std::endl
            << "]";
        return oss.str();
    }

    APR_DECLARE_CLASS()
private:
    /// Albedo refers to Sigma_s / Sigma_t
    ref<Volume> m_sigmat, m_albedo;
    Float m_scale;
};

APR_IMPLEMENT_CLASS_VARIANT(HomogeneousMedium, Medium)
APR_INTERNAL_PLUGIN(HomogeneousMedium, "homogeneous")

} // namespace aspirin