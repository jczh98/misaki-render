#include <iostream>
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
        float int_ior            = props.float_("int_ior", 1.5046f);
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
        Eigen::Vector3f m;
        std::tie(m, bs.pdf) = sample_distr.sample(
            std::copysign(1.0, cos_theta_i) * si.wi, sample);
        if (bs.pdf == 0)
            return { bs, Spectrum::Zero() };
        auto [F, cos_theta_t, eta_it, eta_ti] = fresnel(si.wi.dot(m), m_eta);
        bool selected_r;
        Color3 weight;
        if (has_reflection && has_transmission) {
            selected_r = sample1 <= F;
            weight     = 1.f;
            bs.pdf *= selected_r ? F : (1.f - F);
        } else {
            selected_r = has_reflection;
            weight     = has_reflection ? F : (1.f - F);
        }
        auto selected_t      = !selected_r;
        bs.eta               = selected_r ? 1.f : eta_it;
        bs.sampled_component = selected_r ? (uint32_t) 0 : (uint32_t) 1;
        bs.sampled_type      = selected_r ? +BSDFFlags::GlossyReflection
                                          : +BSDFFlags::GlossyTransmission;
        float dwh_dwo        = 0.f;
        if (selected_r) {
            bs.wo = reflect(si.wi, m);
            weight *= m_specular_reflectance->eval_3(si);
            dwh_dwo = 1.f / (4.f * bs.wo.dot(m));
        } else {
            bs.wo = refract(si.wi, m, cos_theta_t, eta_ti);
            float factor =
                (ctx.mode == TransportMode::Radiance) ? math::sqr(eta_ti) : 1.f;
            weight *= factor;
            dwh_dwo = math::sqr(bs.eta) * bs.wo.dot(m) /
                      math::sqr(si.wi.dot(m) + bs.eta * bs.wo.dot(m));
        }
        if (m_sample_visible)
            weight *= distr.smith_g1(bs.wo, m);
        else
            weight *= distr.G(si.wi, bs.wo, m) * si.wi.dot(m) /
                      (cos_theta_i * Frame::cos_theta(m));
        bs.pdf *= std::abs(dwh_dwo);
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
        bool reflect      = cos_theta_i * cos_theta_o > 0.f;
        float eta         = (cos_theta_i > 0.f ? (m_eta) : (m_inv_eta)),
              inv_eta     = (cos_theta_i > 0.f ? (m_inv_eta) : (m_eta));
        Eigen::Vector3f m = (si.wi + wo * (reflect ? 1.f : eta)).normalized();
        m *= std::copysign(1.f, Frame::cos_theta(m));
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si),
                                     m_alpha_v->eval_1(si), m_sample_visible);
        float D         = distr.eval(m);
        float F         = std::get<0>(fresnel(si.wi.dot(m), m_eta));
        float G         = distr.G(si.wi, wo, m);
        bool eval_r     = has_reflection && reflect;
        bool eval_t     = has_transmission && !reflect;
        Spectrum result = Spectrum::Zero();
        if (eval_r) {
            result = F * D * G * m_specular_reflectance->eval_3(si) /
                     (4.f * std::abs(cos_theta_i));
        }
        if (eval_t) {
            float scale = (ctx.mode == TransportMode::Radiance)
                              ? math::sqr(inv_eta)
                              : 1.f;
            result      = m_specular_transmittance->eval_3(si) *
                     std::abs((scale * (1.f - F) * D * G * eta * eta *
                               si.wi.dot(m) * wo.dot(m)) /
                              (cos_theta_i *
                               math::sqr(si.wi.dot(m) + eta * wo.dot(m))));
        }
        return result;
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
        bool reflect      = cos_theta_i * cos_theta_o > 0.f;
        float eta         = (cos_theta_i > 0.f ? (m_eta) : (m_inv_eta));
        Eigen::Vector3f m = (si.wi + wo * (reflect ? 1.f : eta)).normalized();
        m *= std::copysign(1.f, Frame::cos_theta(m));
        if (si.wi.dot(m) * Frame::cos_theta(si.wi) <= 0.f ||
            wo.dot(m) * Frame::cos_theta(wo) <= 0.f)
            return 0.f;
        float dwh_dwo =
            (reflect ? 1.f / (4.f * wo.dot(m))
                     : (eta * eta * wo.dot(m)) /
                           math::sqr(si.wi.dot(m) + eta * wo.dot(m)));
        MicrofacetDistribution sample_distr(m_type, m_alpha_u->eval_1(si),
                                            m_alpha_v->eval_1(si),
                                            m_sample_visible);
        if (!m_sample_visible)
            sample_distr.scale_alpha(
                1.2f - .2f * std::sqrt(std::abs(Frame::cos_theta(si.wi))));
        float prob = sample_distr.pdf(
            si.wi * std::copysign(1.f, Frame::cos_theta(si.wi)), m);
        if (has_transmission && has_reflection) {
            float F = std::get<0>(fresnel(si.wi.dot(m), m_eta));
            prob *= (reflect ? F : 1.f - F);
        }
        return prob * std::abs(dwh_dwo);
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "RoughDielectric[]";
        return oss.str();
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