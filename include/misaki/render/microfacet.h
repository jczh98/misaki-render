#pragma once

#include "fresnel.h"
#include "misaki/core/fwd.h"
#include "misaki/core/properties.h"

namespace misaki {

namespace microfacet {

inline float eval_ggx(const Eigen::Vector3f &m, float alpha_u, float alpha_v) {
    float cos_theta2  = Frame::cos_theta_2(m);
    float beckman_exp = ((m.x() * m.x() / (alpha_u * alpha_u)) +
                         (m.y() * m.y()) / (alpha_v * alpha_v)) /
                        cos_theta2;
    float root = (1.f + beckman_exp) * cos_theta2;
    return 1.f / (math::Pi<float> * alpha_u * alpha_v * root * root);
}

inline std::pair<Eigen::Vector3f, float>
sample_ggx(const Eigen::Vector3f &wi, const Eigen::Vector2f &sample,
           float alpha_u, float alpha_v) {
    float phi_m = std::atan(alpha_u / alpha_v *
                            std::tan(math::Pi<float> +
                                     2 * math::Pi<float> * sample.y())) +
                  math::Pi<float> * std::floor(2 * sample.y() + 0.5f);
    auto [sin_phi_m, cos_phi_m] = math::sincos(phi_m);
    float cos = cos_phi_m / alpha_u, sin = sin_phi_m / alpha_v;
    auto alpha_sqr        = 1.f / (cos * cos + sin * sin);
    float tan_theta_m_sqr = alpha_sqr * sample.x() / (1.f - sample.x());
    auto cos_theta_m      = 1.f / std::sqrt(1.f + tan_theta_m_sqr);
    float tmp             = 1 + tan_theta_m_sqr / alpha_sqr;
    float pdf = math::InvPi<float> / (alpha_u * alpha_v * cos_theta_m *
                                      cos_theta_m * cos_theta_m * tmp * tmp);
    if (pdf < 1e-20f)
        pdf = 0;
    float sin_theta_m = math::safe_sqrt(1 - cos_theta_m * cos_theta_m);
    return { { sin_theta_m * cos_phi_m, sin_theta_m * sin_phi_m, cos_theta_m },
             pdf };
}

inline float pdf_ggx(const Eigen::Vector3f &m, float alpha_u, float alpha_v) {
    return eval_ggx(m, alpha_u, alpha_v) * Frame::cos_theta(m);
}

} // namespace microfacet

class MicrofacetDistribution {
public:
    enum class Type : uint32_t { Beckmann = 0, GGX = 1 };
    MicrofacetDistribution(const Properties &props, Type type = Type::GGX,
                           float alpha_u = 0.1, float alpha_v = 0.1,
                           bool sample_visible = false)
        : m_type(type), m_alpha_u(alpha_u), m_alpha_v(alpha_v),
          m_sample_visible(sample_visible) {
        if (props.has_property("distribution")) {
            auto distr = props.string("distribution");
            if (distr == "beckmann")
                m_type = Type::Beckmann;
            else if (distr == "ggx")
                m_type = Type::GGX;
            else
                Throw("Specified an invalid microfacet distribution {}", distr);
        }

        if (props.has_property("alpha")) {
            m_alpha_u = m_alpha_v = props.float_("alpha");
            if (props.has_property("alpha_u") || props.has_property("alpha_v"))
                Throw("Microfacet model: please specify"
                      "either 'alpha' or 'alpha_u'/'alpha_v'.");
        } else if (props.has_property("alpha_u") ||
                   props.has_property("alpha_v")) {
            if (!props.has_property("alpha_u") ||
                !props.has_property("alpha_v"))
                Throw("Microfacet model: both 'alpha_u' and 'alpha_v' must be "
                      "specified.");
            if (props.has_property("alpha"))
                Throw("Microfacet model: please specify"
                      "either 'alpha' or 'alpha_u'/'alpha_v'.");
            m_alpha_u = props.float_("alpha_u");
            m_alpha_v = props.float_("alpha_v");
        }

        if (alpha_u == 0.f || alpha_v == 0.f)
            Log(Warn, "Cannot create a microfacet distribution with "
                      "alpha_u/alpha_v=0 (clamped to 10^-4). "
                      "Please use the corresponding smooth reflectance model "
                      "to get zero roughness.");

        m_sample_visible = props.bool_("sample_visible", sample_visible);

        configure();
    }

    MicrofacetDistribution(Type type, float alpha_u, float alpha_v,
                           bool sample_visible = false)
        : m_type(type), m_alpha_u(alpha_u), m_alpha_v(alpha_v),
          m_sample_visible(sample_visible) {
        configure();
    }

    MicrofacetDistribution(Type type, float alpha, bool sample_visible = false)
        : m_type(type), m_alpha_u(alpha), m_alpha_v(alpha),
          m_sample_visible(sample_visible) {
        configure();
    }

    float eval(const Eigen::Vector3f &m) const {
        if (Frame::cos_theta(m) <= 0)
            return 0.0f;
        float result = 0.f;
        switch (m_type) {
            case Type::Beckmann: {
                break;
            }
            case Type::GGX: {
                result = microfacet::eval_ggx(m, m_alpha_u, m_alpha_v);
                break;
            }
            default:
                Throw("Invalid microfacet distribution type!");
                return -1;
        }
        return result * Frame::cos_theta(m) > 1e-20f ? result : 0.f;
    }

    float pdf(const Eigen::Vector3f &wi, const Eigen::Vector3f &m) const {
        return eval(m) * Frame::cos_theta(m);
    }

    std::pair<Eigen::Vector3f, float>
    sample(const Eigen::Vector3f &wi, const Eigen::Vector2f &sample) const {
        switch (m_type) {
            case Type::Beckmann: {
                break;
            }
            case Type::GGX: {
                return microfacet::sample_ggx(wi, sample, m_alpha_u, m_alpha_v);
            }
            default:
                Throw("Invalid microfacet distribution type!");
        }
    }

    float G(const Eigen::Vector3f &wi, const Eigen::Vector3f &wo,
            const Eigen::Vector3f &m) const {
        return smith_g1(wi, m) * smith_g1(wo, m);
    }

    float smith_g1(const Eigen::Vector3f &v, const Eigen::Vector3f &m) const {
        float xy_alpha_2 =
                  math::sqr(m_alpha_u * v.x()) + math::sqr(m_alpha_v * v.y()),
              tan_theta_alpha_2 = xy_alpha_2 / math::sqr(v.z()), result;
        if (xy_alpha_2 == 0.f)
            return 1.f;
        if (v.dot(m) * Frame::cos_theta(v) <= 0.f)
            return 0.f;
        switch (m_type) {
            case Type::Beckmann: {
                float a     = 1.f / std::sqrt(tan_theta_alpha_2),
                      a_sqr = math::sqr(a);
                result      = a >= 1.6f ? 1.f
                                        : (3.535f * a + 2.181f * a_sqr) /
                                         (1.f + 2.276f * a + 2.577f * a_sqr);
                break;
            }
            case Type::GGX: {
                result = 2.f / (1.f + std::sqrt(1.f + tan_theta_alpha_2));
                break;
            }
            default:
                break;
        }
        return result;
    }

    Type type() const { return m_type; }
    // isotropic case
    float alpha() const { return m_alpha_u; }
    float alpha_u() const { return m_alpha_u; }
    float alpha_v() const { return m_alpha_v; }
    bool sample_visible() const { return m_sample_visible; };
    bool is_isotropic() const { return m_alpha_u == m_alpha_v; }
    void scale_alpha(float value) {
        m_alpha_u *= value;
        m_alpha_v *= value;
    }

protected:
    void configure() {
        m_alpha_u = std::max(m_alpha_u, 1e-4f);
        m_alpha_v = std::max(m_alpha_v, 1e-4f);
    }

protected:
    Type m_type;
    float m_alpha_u, m_alpha_v;
    bool m_sample_visible;
};

} // namespace misaki