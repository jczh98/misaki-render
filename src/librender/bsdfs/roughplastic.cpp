#include <misaki/bsdf.h>
#include <misaki/microfacet.h>
#include <misaki/properties.h>
#include <misaki/texture.h>
#include <misaki/warp.h>

namespace misaki {

class RoughPlastic final : public BSDF {
public:
    RoughPlastic(const Properties &props) : BSDF(props) {
        Float int_ior = props.get_float("int_ior", 1.49);
        Float ext_ior = props.get_float("ext_ior", 1.00028);
        m_eta         = int_ior / ext_ior;
        m_diffuse_reflectance =
            props.texture<Texture>("diffuse_reflectance", .5f);
        m_nonlinear = props.get_bool("nonlinear", false);
        m_specular_reflectance =
            props.texture<Texture>("specular_reflectance", 1.f);
        MicrofacetDistribution distr(props);
        m_type           = distr.type();
        m_sample_visible = distr.sample_visible();
        if (!distr.is_isotropic())
            Throw("The 'roughplastic' plugin currently does not support "
                  "anisotropic microfacet distributions!");
        m_alpha = distr.alpha();
        m_components.push_back(+BSDFFlags::GlossyReflection);
        m_components.push_back(+BSDFFlags::DiffuseReflection);
        m_flags = m_components[0] | m_components[1];
        // pre compute
        Float d_mean = m_diffuse_reflectance->mean(), s_mean = 1.f;
        s_mean                     = m_specular_reflectance->mean();
        m_specular_sampling_weight = s_mean / (d_mean + s_mean);
    }

    std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                         const SceneInteraction &si,
                                         Float sample1,
                                         const Vector2 &sample) const override {
        BSDFSample bs;
        Float cos_theta_i = Frame::cos_theta(si.wi);
        bool has_specular = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
             has_diffuse  = ctx.is_enabled(BSDFFlags::DiffuseReflection, 1);
        if ((!has_specular && !has_diffuse) || cos_theta_i <= 0)
            return { bs, Color3(0) };
        Float t_i           = 1.f - std::get<0>(fresnel(cos_theta_i, m_eta));
        Float prob_specular = (1.f - t_i) * m_specular_sampling_weight,
              prob_diffuse  = t_i * (1.f - m_specular_sampling_weight);
        if (has_specular != has_diffuse)
            prob_specular = has_specular ? 1.f : 0.f;
        else
            prob_specular = prob_specular / (prob_specular + prob_diffuse);
        prob_diffuse         = 1.f - prob_specular;
        bool sample_specular = sample1 < prob_specular,
             sample_diffuse  = !sample_specular;
        bs.eta               = 1.f;
        if (sample_specular) {
            MicrofacetDistribution distr(m_type, m_alpha, m_sample_visible);
            Vector3 m            = std::get<0>(distr.sample(si.wi, sample));
            bs.wo                = reflect(si.wi, m);
            bs.sampled_component = 0;
            bs.sampled_type      = +BSDFFlags::GlossyReflection;
        }

        if (sample_diffuse) {
            bs.wo                = warp::square_to_cosine_hemisphere(sample);
            bs.sampled_component = 1;
            bs.sampled_type      = +BSDFFlags::DiffuseReflection;
        }
        Color3 result(0.f);
        bs.pdf = pdf(ctx, si, bs.wo);
        if (bs.pdf <= 0.f)
            return { bs, 0.f };
        result = eval(ctx, si, bs.wo);
        return { bs, result / bs.pdf };
    }

    Color3 eval(const BSDFContext &ctx, const SceneInteraction &si,
                const Vector3 &wo) const override {
        bool has_specular = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
             has_diffuse  = ctx.is_enabled(BSDFFlags::DiffuseReflection, 1);

        Float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);
        Color3 result(0.f);
        if ((!has_specular && !has_diffuse) ||
            !(cos_theta_i > 0.f && cos_theta_o > 0.f))
            return result;
        if (has_specular) {
            MicrofacetDistribution distr(m_type, m_alpha, m_sample_visible);
            // Calculate the reflection half-vector
            Vector3f H = (wo + si.wi).normalized();
            // Evaluate the microfacet normal distribution
            Float D = distr.eval(H);
            // Fresnel term
            Float F = std::get<0>(fresnel(si.wi.dot(H), Float(m_eta)));
            // Smith's shadow-masking function
            Float G = distr.G(si.wi, wo, H);
            // Calculate the specular reflection component
            Color3 value = F * D * G / (4.f * cos_theta_i);
            value *= m_specular_reflectance->eval_3(si.geom);
            result += value;
        }

        if (has_diffuse) {
            Float t_i   = 1.f - std::get<0>(fresnel(cos_theta_i, m_eta)),
                  t_o   = 1.f - std::get<0>(fresnel(cos_theta_o, m_eta)),
                  fdr   = fresnel_diffuse_reflectance(m_eta);
            Color3 diff = m_diffuse_reflectance->eval_3(si.geom);
            diff /= 1.f - (m_nonlinear ? (diff * fdr) : Color3(fdr));
            result += diff * (math::InvPi<Float> * (1.f / m_eta / m_eta) *
                              cos_theta_o * t_i * t_o);
        }
        return result;
    }

    Float pdf(const BSDFContext &ctx, const SceneInteraction &si,
              const Vector3 &wo) const override {
        bool has_specular = ctx.is_enabled(BSDFFlags::DeltaReflection, 0),
             has_diffuse  = ctx.is_enabled(BSDFFlags::DiffuseReflection, 1);
        Float cos_theta_i = Frame::cos_theta(si.wi),
              cos_theta_o = Frame::cos_theta(wo);

        if ((!has_specular && !has_diffuse) ||
            !(cos_theta_i > 0.f && cos_theta_o > 0.f))
            return 0.f;

        Float t_i = 1.f - std::get<0>(fresnel(cos_theta_i, m_eta));

        // Determine which component should be sampled
        Float prob_specular = (1.f - t_i) * m_specular_sampling_weight,
              prob_diffuse  = t_i * (1.f - m_specular_sampling_weight);

        if (has_specular != has_diffuse)
            prob_specular = has_specular ? 1.f : 0.f;
        else
            prob_specular = prob_specular / (prob_specular + prob_diffuse);
        prob_diffuse = 1.f - prob_specular;

        Vector3f H = (wo + si.wi).normalized();

        MicrofacetDistribution distr(m_type, m_alpha, m_sample_visible);
        Float result = 0.f;
        if (m_sample_visible)
            result =
                distr.eval(H) * distr.smith_g1(si.wi, H) / (4.f * cos_theta_i);
        else
            result = distr.pdf(si.wi, H) / (4.f * wo.dot(H));
        result *= prob_specular;
        result += prob_diffuse * warp::square_to_cosine_hemisphere_pdf(wo);
        return result;
    }

    MSK_DECL_COMP(BSDF)
private:
    MicrofacetDistribution::Type m_type;
    Float m_eta;
    Float m_alpha;
    Float m_specular_sampling_weight;
    bool m_nonlinear;
    std::shared_ptr<Texture> m_specular_reflectance;
    std::shared_ptr<Texture> m_diffuse_reflectance;
    bool m_sample_visible;
};

MSK_EXPORT_PLUGIN(RoughPlastic)

} // namespace misaki