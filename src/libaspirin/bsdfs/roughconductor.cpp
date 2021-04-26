#include <aspirin/bsdf.h>
#include <aspirin/microfacet.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

class RoughConductor final : public BSDF {
public:
    RoughConductor(const Properties &props) : BSDF(props) {
        if (props.has_property("eta")) {
            m_eta = props.texture<Texture>("eta", 0.f);
            m_k   = props.texture<Texture>("k", 1.f);
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
        m_sample_visible = props.get_bool("sample_visible", false);
        if (props.has_property("alpha_u") || props.has_property("alpha_v")) {
            if (!props.has_property("alpha_u") ||
                !props.has_property("alpha_v"))
                Throw("Microfacet model: both 'alpha_u' and 'alpha_v' must be "
                      "specified.");
            if (props.has_property("alpha"))
                Throw("Microfacet model: please specify"
                      "either 'alpha' or 'alpha_u'/'alpha_v'.");
            m_alpha_u = props.texture<Texture>("alpha_u");
            m_alpha_v = props.texture<Texture>("alpha_v");
        } else {
            m_alpha_u = m_alpha_v = props.texture<Texture>("alpha", 0.1f);
        }
        m_specular_reflectance =
            props.texture<Texture>("specular_reflectance", 1.f);
        m_flags = +BSDFFlags::GlossyReflection;
        m_components.clear();
        m_components.push_back(m_flags);
    }

    std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                         const SceneInteraction &si,
                                         Float sample1,
                                         const Vector2 &sample) const override {
        BSDFSample bs;
        Float cos_theta_i = Frame::cos_theta(si.wi);
        if (!ctx.is_enabled(BSDFFlags::GlossyReflection) || cos_theta_i <= 0.f)
            return { bs, 0.f };
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si.geom),
                                     m_alpha_v->eval_1(si.geom),
                                     m_sample_visible);
        Vector3 m;
        std::tie(m, bs.pdf)  = distr.sample(si.wi, sample);
        bs.wo                = reflect(si.wi, m);
        bs.eta               = 1.f;
        bs.sampled_component = 0;
        bs.sampled_type      = +BSDFFlags::GlossyReflection;
        if (!(bs.pdf != 0.f && Frame::cos_theta(bs.wo) > 0.f))
            return { bs, 0.f };
        Color3 weight;
        if (m_sample_visible)
            weight = distr.smith_g1(bs.wo, m);
        else
            weight = distr.G(si.wi, bs.wo, m) * si.wi.dot(m) /
                     (cos_theta_i * Frame::cos_theta(m));
        // Jacobian of the half-direction mapping
        bs.pdf /= 4.f * bs.wo.dot(m);
        Color3 F = fresnel_conductor(si.wi.dot(m), m_eta->eval_3(si.geom),
                                     m_k->eval_3(si.geom));
        return { bs, (F * weight) };
    }

    Color3 eval(const BSDFContext &ctx, const SceneInteraction &si,
                const Vector3 &wo) const override {
        Float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        if (!ctx.is_enabled(BSDFFlags::GlossyReflection) ||
            !(cos_theta_i > 0.f && cos_theta_o > 0.f))
            return 0.f;
        Vector3 H = (wo + si.wi).normalized();
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si.geom),
                                     m_alpha_v->eval_1(si.geom),
                                     m_sample_visible);
        Float D = distr.eval(H);
        if (D == 0)
            return 0.f;
        Float G       = distr.G(si.wi, wo, H);
        Color3 result = D * G / (4.f * Frame::cos_theta(si.wi));
        Color3 F      = fresnel_conductor(si.wi.dot(H), m_eta->eval_3(si.geom),
                                     m_k->eval_3(si.geom));
        result *= m_specular_reflectance->eval_3(si.geom);
        return F * result;
    }

    Float pdf(const BSDFContext &ctx, const SceneInteraction &si,
              const Vector3 &wo) const override {
        Float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        Vector3 m         = (wo + si.wi).normalized();
        if (!ctx.is_enabled(BSDFFlags::GlossyReflection) ||
            !(cos_theta_i > 0.f && cos_theta_o > 0.f && si.wi.dot(m) > 0.f &&
              wo.dot(m) > 0.f)) {
            return 0.f;
        }
        MicrofacetDistribution distr(m_type, m_alpha_u->eval_1(si.geom),
                                     m_alpha_v->eval_1(si.geom),
                                     m_sample_visible);
        if (m_sample_visible) {
            return distr.eval(m) * distr.smith_g1(si.wi, m) /
                   (4.f * cos_theta_i);
        } else {
            return distr.pdf(si.wi, m) / (4.f * wo.dot(m));
        }
    }

    MSK_DECL_COMP(BSDF)
private:
    MicrofacetDistribution::Type m_type;
    std::shared_ptr<Texture> m_alpha_u, m_alpha_v;
    std::shared_ptr<Texture> m_eta;
    std::shared_ptr<Texture> m_k;
    std::shared_ptr<Texture> m_specular_reflectance;
    bool m_sample_visible;
};

MSK_EXPORT_PLUGIN(RoughConductor)
} // namespace aspirin