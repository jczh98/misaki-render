#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/core/warp.h>
#include <misaki/render/bsdf.h>
#include <misaki/render/fresnel.h>
#include <misaki/render/microfacet.h>
#include <misaki/render/texture.h>

namespace misaki {

class RoughDielectric final : public BSDF {
public:
    RoughDielectric(const Properties &props) : BSDF(props) {
        m_specular_reflectance   = props.texture("specular_reflectance", 1.f);
        m_specular_transmittance = props.texture("specular_transmittance", 1.f);
        float int_ior            = props.float_("int_ior", 1.49);
        float ext_ior            = props.float_("ext_ior", 1.00028);
        if (int_ior < 0.f || ext_ior < 0.f || int_ior == ext_ior)
            Throw("The interior and exterior indices of "
                  "refraction must be positive and differ!");
        m_eta     = int_ior / ext_ior;
        m_inv_eta = ext_ior / int_ior;
        if (props.has_property("distribution")) {
            std::string distr = string::to_lower(props.string("distribution"));
            if (distr == "beckmann")
                m_type = MicrofacetDistribution::Type::Beckmann;
            else if (distr == "ggx")
                m_type = MicrofacetDistribution::Type::GGX;
            else
                Throw("Specified an invalid distribution \"%s\", must be "
                      "\"beckmann\" or \"ggx\"!",
                      distr.c_str());
        } else {
            m_type = MicrofacetDistribution::Type::Beckmann;
        }
        m_sample_visible = props.bool_("sample_visible", false);
        if (props.has_property("alpha_u") || props.has_property("alpha_v")) {
            if (!props.has_property("alpha_u") ||
                !props.has_property("alpha_v"))
                Throw("Microfacet model: both 'alpha_u' and 'alpha_v' must be "
                      "specified.");
            if (props.has_property("alpha"))
                Throw("Microfacet model: please specify"
                      "either 'alpha' or 'alpha_u'/'alpha_v'.");
            m_alpha_u = props.texture("alpha_u");
            m_alpha_v = props.texture("alpha_v");
        } else {
            m_alpha_u = m_alpha_v = props.texture("alpha", 0.1f);
        }
        m_components.push_back(+BSDFFlags::GlossyReflection);
        m_components.push_back(+BSDFFlags::GlossyTransmission);
        m_flags = m_components[0] | m_components[1];
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx, const SceneInteraction &si, float sample1,
           const Eigen::Vector2f &sample) const override {
        bool has_reflection = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
             has_transmission =
                 ctx.is_enabled(BSDFFlags::GlossyTransmission, 1);
        BSDFSample bs;
        if (!has_reflection && !has_transmission)
            return { bs, Spectrum::Zero() };
        float cos_theta_i = Frame::cos_theta(si.wi);
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si),
                                     m_alpha_v->eval_1(si), m_sample_visible);
        MicrofacetDistribution sample_distr(distr);
        if (!m_sample_visible)
            sample_distr.scale_alpha(1.2f -
                                     .2f * std::sqrt(std::abs(cos_theta_i)));
        auto [m, micro_pdf] = sample_distr.sample(
            std::copysign(1.f, Frame::cos_theta(si.wi)) * si.wi, sample);
        if (micro_pdf == 0.f)
            return { bs, Spectrum::Zero() };
        auto [F, cos_theta_t, eta_it, eta_ti] = fresnel(si.wi.dot(m), m_eta);

        bool sample_reflection = false;
        Spectrum weight;
        if (has_reflection && has_transmission) {
            if (sample1 > F)
                sample_reflection = false;
        } else {
            weight = has_reflection ? F : (1 - F);
        }

        if (sample_reflection) {
            bs.wo                = reflect(si.wi, m);
            bs.eta               = 1.f;
            bs.sampled_component = 0;
            bs.sampled_type      = +BSDFFlags::GlossyReflection;
            if (cos_theta_i * Frame::cos_theta(bs.wo) <= 0)
                return { bs, Spectrum::Zero() };
            weight *= m_specular_reflectance->eval_3(si);
        } else {
            if (cos_theta_t == 0)
                return { bs, Spectrum::Zero() };
            bs.wo                = refract(si.wi, m, m_eta, cos_theta_t);
            bs.eta               = cos_theta_t < 0 ? m_eta : m_inv_eta;
            bs.sampled_component = 1;
            bs.sampled_type      = +BSDFFlags::GlossyTransmission;
            if (cos_theta_i * Frame::cos_theta(bs.wo) >= 0)
                return { bs, Spectrum::Zero() };

            float factor = (ctx.mode == TransportMode::Radiance)
                               ? (cos_theta_t < 0 ? m_inv_eta : m_eta)
                               : 1.0f;

            weight *= m_specular_transmittance->eval_3(si) * (factor * factor);
        }
        if (m_sample_visible)
            weight *= distr.smith_g1(bs.wo, m);
        else
            weight *=
                std::abs(distr.eval(m) * distr.G(si.wi, bs.wo, m) *
                         si.wi.dot(m) / (micro_pdf * Frame::cos_theta(si.wi)));
        bs.pdf = pdf(ctx, si, bs.wo);
        return { bs, weight };
    }

    Spectrum eval(const BSDFContext &ctx, const SceneInteraction &si,
                  const Eigen::Vector3f &wo) const override {
        float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        if (cos_theta_i == 0.f)
            return Spectrum::Zero();
        bool has_reflection = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
             has_transmission =
                 ctx.is_enabled(BSDFFlags::GlossyTransmission, 1);
        bool reflect = cos_theta_i * cos_theta_o > 0.f;
        Eigen::Vector3f H;
        if (reflect) {
            if (!has_reflection)
                return Spectrum::Zero();
            H = (wo + si.wi).normalized();
        } else {
            if (!has_transmission)
                return Spectrum::Zero();

            float eta = Frame::cos_theta(si.wi) > 0 ? m_eta : m_inv_eta;

            H = (wo * eta + si.wi).normalized();
        }
        H *= std::copysign(1.f, Frame::cos_theta(H));
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si),
                                     m_alpha_v->eval_1(si), m_sample_visible);
        const float D = distr.eval(H);
        if (D == 0)
            return Spectrum::Zero();
        const float F = std::get<0>(fresnel(si.wi.dot(H), m_eta));
        const float G = distr.G(si.wi, wo, H);
        if (reflect) {
            return F * D * G * m_specular_reflectance->eval_3(si) /
                   (4.f * std::abs(cos_theta_i));
        } else {
            float eta = Frame::cos_theta(si.wi) > 0.0f ? m_eta : m_inv_eta;

            float sqrt_denom = si.wi.dot(H) + eta * wo.dot(H);
            float value =
                ((1 - F) * D * G * eta * eta * si.wi.dot(H) * wo.dot(H)) /
                (Frame::cos_theta(si.wi) * sqrt_denom * sqrt_denom);

            float factor =
                (ctx.mode == TransportMode::Radiance)
                    ? (Frame::cos_theta(si.wi) > 0 ? m_inv_eta : m_eta)
                    : 1.0f;

            return m_specular_transmittance->eval_3(si) *
                   std::abs(value * factor * factor);
        }
    }

    float pdf(const BSDFContext &ctx, const SceneInteraction &si,
              const Eigen::Vector3f &wo) const override {
        float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        if (cos_theta_i == 0.f)
            return 0.f;
        bool has_reflection = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
             has_transmission =
                 ctx.is_enabled(BSDFFlags::GlossyTransmission, 1);
        if (!has_reflection && !has_transmission)
            return 0.f;
        bool reflect = cos_theta_i * cos_theta_o > 0.f;
        Eigen::Vector3f H;
        float dwh_dwo;
        if (reflect) {
            if (!has_reflection)
                return 0.f;
            H       = (wo + si.wi).normalized();
            dwh_dwo = 1.0f / (4.0f * wo.dot(H));
        } else {
            if (!has_transmission)
                return 0.f;

            float eta = Frame::cos_theta(si.wi) > 0 ? m_eta : m_inv_eta;

            H                = (wo * eta + si.wi).normalized();
            float sqrt_denom = si.wi.dot(H) + eta * wo.dot(H);
            dwh_dwo = (eta * eta * wo.dot(H)) / (sqrt_denom * sqrt_denom);
        }
        H *= std::copysign(1.f, Frame::cos_theta(H));
        MicrofacetDistribution sample_distr(m_type, m_alpha_u->eval_1(si),
                                            m_alpha_v->eval_1(si),
                                            m_sample_visible);
        if (!m_sample_visible)
            sample_distr.scale_alpha(
                1.2f - .2f * std::sqrt(std::abs(Frame::cos_theta(si.wi))));
        float prob = sample_distr.pdf(
            si.wi * std::copysign(1.f, Frame::cos_theta(si.wi)), H);
        if (has_transmission && has_reflection) {
            float F = std::get<0>(fresnel(si.wi.dot(H), m_eta));
            prob *= (reflect ? F : 1.f - F);
        }
        return std::abs(prob * dwh_dwo);
    }

    MSK_DECLARE_CLASS()
private:
    ref<Texture> m_specular_reflectance;
    ref<Texture> m_specular_transmittance;
    ref<Texture> m_alpha_u, m_alpha_v;
    MicrofacetDistribution::Type m_type;
    float m_eta, m_inv_eta;
    bool m_sample_visible;
};

MSK_IMPLEMENT_CLASS(RoughDielectric, BSDF)
MSK_REGISTER_INSTANCE(RoughDielectric, "roughdielectric")

} // namespace misaki