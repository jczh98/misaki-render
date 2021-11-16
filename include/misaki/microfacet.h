#pragma once

#include "fresnel.h"
#include "fwd.h"
#include "properties.h"

namespace misaki {

namespace microfacet {

float gtr1(float cos_theta_h, float alpha) noexcept {
    const float a2 = alpha * alpha;
    const float t  = 1 + (a2 - 1) * math::sqr(cos_theta_h);
    return (a2 - 1) / (math::Pi<float> * std::log(a2) * t);
}

float gtr2(float cos_theta_h, float alpha) noexcept {
    const float a2 = alpha * alpha;
    const float t  = 1 + (a2 - 1) * math::sqr(cos_theta_h);
    return a2 / (math::Pi<float> * t * t);
}

float gtr2_aniso(float sin_theta_h, float cos_theta_h, float sin_phi_h,
                 float cos_phi_h, float ax, float ay) noexcept {
    const float t  = math::sqr(cos_phi_h / ax) + math::sqr(sin_phi_h / ay);
    const float t2 = math::sqr(sin_theta_h) * t + math::sqr(cos_theta_h);
    return 1.f / (math::Pi<float> * ax * ay * math::sqr(t2));
}

float smith_g1_ggx(float tan_theta, float alpha) noexcept {
    if (!tan_theta)
        return 1.f;
    const float t = tan_theta * alpha;
    return 2.f / (1.f + std::sqrt(1 + t * t));
}

float smith_g1_ggx_aniso(float tan_theta, float sin_phi, float cos_phi,
                         float ax, float ay) noexcept {
    const float t       = math::sqr(ax * cos_phi) + math::sqr(ay * sin_phi);
    const float sqr_val = 1 + t * math::sqr(tan_theta);
    const float lambda  = -float(0.5) + float(0.5) * std::sqrt(sqr_val);
    return 1 / (1 + lambda);
}

Eigen::Vector3f sample_gtr1(float alpha, const Eigen::Vector2f &sample) noexcept {
    const float phi       = 2 * math::Pi<float> * sample.x();
    const float cos_theta = std::sqrt(
        (std::pow(alpha, 2 - 2 * sample.y()) - 1) / (math::sqr(alpha) - 1));
    const float sin_theta = math::safe_sqrt(1.f - cos_theta * cos_theta);
    return math::normalize(Eigen::Vector3f(sin_theta * std::cos(phi),
                                   sin_theta * std::sin(phi), cos_theta));
}

Eigen::Vector3f sample_gtr2_aniso(float ax, float ay, const Eigen::Vector2f &sample) noexcept {
    float sin_phi_h = ay * std::sin(2 * math::Pi<float> * sample.x());
    float cos_phi_h = ax * std::cos(2 * math::Pi<float> * sample.x());
    const float nor =
        1 / std::sqrt(math::sqr(sin_phi_h) + math::sqr(cos_phi_h));
    sin_phi_h *= nor;
    cos_phi_h *= nor;

    const float A = math::sqr(cos_phi_h / ax) + math::sqr(sin_phi_h / ay);
    const float cos_theta_h =
        std::sqrt(A * (1 - sample.y()) / ((1 - A) * sample.y() + A));
    const float sin_theta_h =
        std::sqrt(std::max<float>(0, 1 - math::sqr(cos_theta_h)));

    return math::normalize(
        Eigen::Vector3f(sin_theta_h * cos_phi_h, sin_theta_h * sin_phi_h, cos_theta_h));
}

} // namespace microfacet

class MicrofacetDistribution {
public:
    enum class Type : uint32_t { Beckmann = 0, GGX = 1 };
    MicrofacetDistribution(const Properties &props, Type type = Type::Beckmann,
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
            m_alpha_u = m_alpha_v = props.get_float("alpha");
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
            m_alpha_u = props.get_float("alpha_u");
            m_alpha_v = props.get_float("alpha_v");
        }

        if (alpha_u == 0.f || alpha_v == 0.f)
            Log(Warn, "Cannot create a microfacet distribution with "
                      "alpha_u/alpha_v=0 (clamped to 10^-4). "
                      "Please use the corresponding smooth reflectance model "
                      "to get zero roughness.");

        m_sample_visible = props.get_bool("sample_visible", sample_visible);

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
        float alpha_uv = m_alpha_u * m_alpha_v, cos_theta = Frame::cos_theta(m),
              cos_theta_2 = cos_theta * cos_theta,
              temporal    = (m.x() / m_alpha_u) * (m.x() / m_alpha_u) +
                         (m.y() / m_alpha_v) * (m.y() / m_alpha_v),
              result;
        switch (m_type) {
            case Type::Beckmann: {
                result =
                    std::exp(-temporal / cos_theta_2) /
                    (math::Pi<float> * alpha_uv * cos_theta_2 * cos_theta_2);
                break;
            }
            case Type::GGX: {
                result = 1.f / (math::Pi<float> * alpha_uv *
                                (temporal + m.z() * m.z()) *
                                (temporal + m.z() * m.z()));
                break;
            }
            default:
                break;
        }
        return result * cos_theta > 1e-20f ? result : 0.f;
    }

    float pdf(const Eigen::Vector3f &wi, const Eigen::Vector3f &m) const {
        float result = eval(m);
        if (m_sample_visible)
            result *=
                smith_g1(wi, m) * std::abs(wi.dot(m)) / Frame::cos_theta(wi);
        else
            result *= Frame::cos_theta(m);
        return result;
    }

    std::pair<Eigen::Vector3f, float> sample(const Eigen::Vector3f &wi,
                                     const Eigen::Vector2f &sample) const {
        if (!m_sample_visible) {
            float sin_phi, cos_phi, cos_theta, cos_theta_2, alpha_2, pdf;
            if (is_isotropic()) {
                float temp = (2.f * math::Pi<float>) *sample.y();
                sin_phi    = std::sin(temp);
                cos_phi    = std::cos(temp);
                alpha_2    = m_alpha_u * m_alpha_u;
            } else {
                float ratio = m_alpha_v / m_alpha_u,
                      tmp   = ratio * tan((2.f * math::Pi<float>) *sample.y());
                cos_phi     = 1.f / std::sqrt(tmp * tmp + 1.f);
                cos_phi =
                    cos_phi * std::copysign(1.f, abs(sample.y() - .5f) - .25f);

                sin_phi = cos_phi * tmp;
                alpha_2 = 1.f / (math::sqr(cos_phi / m_alpha_u) +
                                 math::sqr(sin_phi / m_alpha_v));
            }
            switch (m_type) {
                case Type::Beckmann: {
                    // Beckmann distribution function for Gaussian random
                    // surfaces
                    cos_theta =
                        1.f /
                        std::sqrt(1.f - alpha_2 * std::log(1.f - sample.x()));
                    cos_theta_2 = math::sqr(cos_theta);

                    // Compute probability density of the sampled position
                    float cos_theta_3 =
                        std::max(cos_theta_2 * cos_theta, 1e-20f);
                    pdf = (1.f - sample.x()) / (math::Pi<float> * m_alpha_u *
                                                m_alpha_v * cos_theta_3);
                }
                case Type::GGX: {
                    // GGX / Trowbridge-Reitz distribution function
                    float tan_theta_m_2 =
                        alpha_2 * sample.x() / (1.f - sample.x());
                    cos_theta   = 1.f / std::sqrt(1.f + tan_theta_m_2);
                    cos_theta_2 = math::sqr(cos_theta);

                    // Compute probability density of the sampled position
                    float temp = 1.f + tan_theta_m_2 / alpha_2,
                          cos_theta_3 =
                              std::max(cos_theta_2 * cos_theta, 1e-20f);
                    pdf = 1.f / (math::Pi<float> * m_alpha_u * m_alpha_v *
                                 cos_theta_3 * math::sqr(temp));
                }
                default:
                    break;
            }
            float sin_theta = std::sqrt(1.f - cos_theta_2);
            return { { cos_phi * sin_theta, sin_phi * sin_theta, cos_theta },
                     pdf };
        } else {
            Log(Warn, "sample visible not supported");
        }
    }

    float G(const Eigen::Vector3f &wi, const Eigen::Vector3f &wo, const Eigen::Vector3f &m) const {
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