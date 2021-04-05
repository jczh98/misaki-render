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
        m_sigma_a        = props.texture<Texture>("albedo", 0.75f);
        m_sigma_s        = props.texture<Texture>("sigma_t", 1.f);
        m_sigma_t        = m_sigma_s + m_sigma_a;
        m_scale          = props.get_float("scale", 1.0f);
    }

    std::pair<MediumInteraction, Float>
    sample_interaction(const Ray &ray, Float sample,
                       uint32_t channel) const override {
        Float maxt = std::min(ray.maxt, std::numeric_limits<Float>::max()),
              sampled_distance;

        sampled_distance = -std::log((1 - sample) / m_sigma_t[channel]);
        MediumInteraction mi;
        Float pdf;
        if (sampled_distance < ray.maxt - ray.mint) {
            mi.t       = sampled_distance + ray.mint;
            mi.p       = ray(mi.t);
            mi.sigma_a = m_sigma_a;
            mi.sigma_s = m_sigma_s;
            pdf = ((m_sigma_t * (-sampled_distance)).exp() * m_sigma_t).mean();
        } else {
            sampled_distance = ray.maxt - ray.mint;
            pdf              = (m_sigma_t * (-sampled_distance)).exp().mean();
        }
        mi.transmittance = (m_sigma_t * (-sampled_distance)).exp();
        mi.medium        = this;
        return { mi, pdf };
    }

    Spectrum eval_transmittance(const Ray &ray) const override {
        Float t = std::min(ray.maxt, std::numeric_limits<float>::max());
        return (m_sigma_t * (-t)).exp();
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "HomogeneousMedium[" << std::endl
            << "  albedo  = " << string::indent(m_sigma_s) << std::endl
            << "  sigma_t = " << string::indent(m_sigma_a) << std::endl
            << "  scale   = " << m_scale << std::endl
            << "]";
        return oss.str();
    }

    APR_DECLARE_CLASS()
private:
    Spectrum m_sigma_s, m_sigma_a, m_sigma_t;
    Float m_scale;
};

APR_IMPLEMENT_CLASS_VARIANT(HomogeneousMedium, Medium)
APR_INTERNAL_PLUGIN(HomogeneousMedium, "homogeneous")

} // namespace aspirin