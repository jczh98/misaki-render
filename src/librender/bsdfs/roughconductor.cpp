#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/core/warp.h>
#include <misaki/render/bsdf.h>
#include <misaki/render/fresnel.h>
#include <misaki/render/microfacet.h>
#include <misaki/render/texture.h>

namespace misaki {

class RoughConductor final : public BSDF {
public:
    RoughConductor(const Properties &props) : BSDF(props) {
        if (props.has_property("eta")) {
            m_eta = props.texture("eta", 0.f);
            m_k   = props.texture("k", 1.f);
        }
        if (props.has_property("distribution")) {
            std::string distr = props.string("distribution");
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
        m_specular_reflectance = props.texture("specular_reflectance", 1.f);
        m_flags                = +BSDFFlags::GlossyReflection;
        m_components.clear();
        m_components.push_back(m_flags);
    }

    std::pair<BSDFSample, Spectrum>
    sample(const BSDFContext &ctx, const SceneInteraction &si, float sample1,
           const Eigen::Vector2f &sample) const override {
        BSDFSample bs;
        float cos_theta_i = Frame::cos_theta(si.wi);
        if (!ctx.is_enabled(BSDFFlags::GlossyReflection) || cos_theta_i <= 0.f)
            return { bs, Spectrum::Zero() };
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si),
                                     m_alpha_v->eval_1(si), m_sample_visible);
        Eigen::Vector3f m;
        std::tie(m, bs.pdf)  = distr.sample(si.wi, sample);
        bs.wo                = reflect(si.wi, m);
        bs.eta               = 1.f;
        bs.sampled_component = 0;
        bs.sampled_type      = +BSDFFlags::GlossyReflection;
        if (!(bs.pdf != 0.f && Frame::cos_theta(bs.wo) > 0.f))
            return { bs, Spectrum::Zero() };
        Color3 weight;
        if (m_sample_visible)
            weight = distr.smith_g1(bs.wo, m);
        else
            weight = distr.G(si.wi, bs.wo, m) * si.wi.dot(m) /
                     (cos_theta_i * Frame::cos_theta(m));
        // Jacobian of the half-direction mapping
        bs.pdf /= 4.f * bs.wo.dot(m);
        Color3 F =
            fresnel_conductor(si.wi.dot(m), m_eta->eval_3(si), m_k->eval_3(si));
        return { bs, (F * weight) };
    }

    Spectrum eval(const BSDFContext &ctx, const SceneInteraction &si,
                  const Eigen::Vector3f &wo) const override {
        float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        if (!ctx.is_enabled(BSDFFlags::GlossyReflection) ||
            !(cos_theta_i > 0.f && cos_theta_o > 0.f))
            return Spectrum::Zero();
        Eigen::Vector3f H = (wo + si.wi).normalized();
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si),
                                     m_alpha_v->eval_1(si), m_sample_visible);
        float D = distr.eval(H);
        if (D == 0)
            return Spectrum::Zero();
        float G      = distr.G(si.wi, wo, H);
        float result = D * G / (4.f * Frame::cos_theta(si.wi));
        Spectrum F =
            fresnel_conductor(si.wi.dot(H), m_eta->eval_3(si), m_k->eval_3(si));
        return F * m_specular_reflectance->eval_3(si) * result;
    }

    float pdf(const BSDFContext &ctx, const SceneInteraction &si,
              const Eigen::Vector3f &wo) const override {
        float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        Eigen::Vector3f m = (wo + si.wi).normalized();
        if (!ctx.is_enabled(BSDFFlags::GlossyReflection) ||
            !(cos_theta_i > 0.f && cos_theta_o > 0.f && si.wi.dot(m) > 0.f &&
              wo.dot(m) > 0.f)) {
            return 0.f;
        }
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si),
                                     m_alpha_v->eval_1(si), m_sample_visible);
        if (m_sample_visible) {
            return distr.eval(m) * distr.smith_g1(si.wi, m) /
                   (4.f * cos_theta_i);
        } else {
            return distr.pdf(si.wi, m) / (4.f * wo.dot(m));
        }
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "RoughConductor[]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
protected:
    MicrofacetDistribution::Type m_type;
    ref<Texture> m_alpha_u, m_alpha_v;
    ref<Texture> m_eta;
    ref<Texture> m_k;
    ref<Texture> m_specular_reflectance;
    bool m_sample_visible;
};

MSK_IMPLEMENT_CLASS(RoughConductor, BSDF)
MSK_REGISTER_INSTANCE(RoughConductor, "roughconductor")

} // namespace misaki