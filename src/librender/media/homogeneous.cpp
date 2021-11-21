#include <iostream>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/interaction.h>
#include <misaki/render/medium.h>
#include <misaki/render/texture.h>
#include <misaki/render/volume.h>
namespace misaki {

class HomogeneousMedium final : public Medium {
public:
    explicit HomogeneousMedium(const Properties &props) : Medium(props) {
        m_is_homogeneous = true;
        m_sigma_a        = props.color("sigma_a", Spectrum::Constant(1.f));
        m_sigma_s        = props.color("sigma_s", Spectrum::Constant(1.f));
        m_sigma_t        = m_sigma_s + m_sigma_a;
        m_scale          = props.float_("scale", 1.0f);
    }

    std::pair<bool, MediumSample>
    sample_distance(const Ray &ray, float sample,
                    uint32_t channel) const override {
        float sampled_distance = -std::log(1 - sample) / m_sigma_t[channel];
        bool success = true;
        MediumSample ms;
        if (sampled_distance < ray.maxt - ray.mint) {
            ms.t       = sampled_distance + ray.mint;
            ms.p       = ray(ms.t);
            ms.sigma_a = m_sigma_a;
            ms.sigma_s = m_sigma_s;
            if (ms.p == ray.o) {
                ms.t = math::Infinity<float>;
                ms.pdf  = (m_sigma_t * (-sampled_distance)).exp().mean();
                success = false;
            } else
                ms.pdf = ((m_sigma_t * (-sampled_distance)).exp() * m_sigma_t)
                          .mean();
        } else {
            ms.t             = math::Infinity<float>;
            sampled_distance = ray.maxt - ray.mint;
            ms.pdf              = (m_sigma_t * (-sampled_distance)).exp().mean();
            success          = false;
        }
        ms.transmittance = (m_sigma_t * (-sampled_distance)).exp();
        if (ms.transmittance.maxCoeff() < 1e-20)
            ms.transmittance = Spectrum::Zero();
        return { success, ms };
    }

    Spectrum eval_transmittance(const Ray &ray) const override {
        float neg_length = ray.mint - ray.maxt;
        return (m_sigma_t * neg_length).exp();
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "HomogeneousMedium[" << std::endl
            << "  sigma_a = " << string::indent(m_sigma_a) << std::endl
            << "  sigma_t = " << string::indent(m_sigma_s) << std::endl
            << "  scale   = " << m_scale << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    Spectrum m_sigma_s, m_sigma_a, m_sigma_t;
    float m_scale;
};

MSK_IMPLEMENT_CLASS(HomogeneousMedium, Medium)
MSK_REGISTER_INSTANCE(HomogeneousMedium, "homogeneous")

} // namespace misaki