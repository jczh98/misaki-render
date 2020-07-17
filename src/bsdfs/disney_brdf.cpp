#include <misaki/render/bsdf.h>
#include <misaki/render/logger.h>
#include <misaki/render/microfacet.h>
#include <misaki/render/properties.h>
#include <misaki/render/texture.h>
#include <misaki/render/warp.h>

namespace misaki::render {

class DisneyPrincipledBRDF final : public BSDF {
 public:
  DisneyPrincipledBRDF(const Properties &props) : BSDF(props) {
    m_base_color = props.texture<Texture>("base_color", 0.5f);
    m_subsurface = props.texture<Texture>("subsurface", 0.5f);
    m_metallic = props.texture<Texture>("metallic", 0.5f);
    m_specular = props.texture<Texture>("specular", 0.5f);
    m_specular_tint = props.texture<Texture>("specular_tint", 0.5f);
    m_roughness = props.texture<Texture>("roughness", 0.5f);
    m_anisotropic = props.texture<Texture>("anisotropic", 0.5f);
    m_sheen = props.texture<Texture>("sheen", 0.5f);
    m_sheen_tint = props.texture<Texture>("sheen_tint", 0.5f);
    m_clearcoat = props.texture<Texture>("clearcoat", 0.5f);
    m_clearcoat_gloss = props.texture<Texture>("clearcoat_gloss", 0.5f);
    m_components.push_back(+BSDFFlags::GlossyReflection);
    m_components.push_back(+BSDFFlags::DiffuseReflection);
    m_flags = m_components[0] | m_components[1];
  }

  std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                       const SceneInteraction &si,
                                       Float sample1,
                                       const Vector2 &sample) const override {
    Float cos_theta_i = Frame::cos_theta(si.wi);
    BSDFSample bs;
    if (cos_theta_i <= 0.f || !ctx.is_enabled(BSDFFlags::DiffuseReflection)) return {bs, 0.f};
  }

  Color3 eval(const BSDFContext &ctx,
              const SceneInteraction &si,
              const Vector3 &wo) const override {
    bool has_specular = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
         has_diffuse = ctx.is_enabled(BSDFFlags::DiffuseReflection, 1);

    Float cos_theta_i = Frame::cos_theta(si.wi),
          cos_theta_o = Frame::cos_theta(wo);
    Color3 result(0.f);
    if ((!has_specular && !has_diffuse) || !(cos_theta_i > 0.f && cos_theta_o > 0.f))
      return result;
    // Eval textures
    auto base_color = m_base_color->eval_3(si.geom);
    auto metallic = m_metallic->eval_1(si.geom);
    auto subsurface = m_subsurface->eval_1(si.geom);
    auto specular = m_specular->eval_1(si.geom);
    auto specular_tint = m_specular_tint->eval_1(si.geom);
    auto roughness = m_roughness->eval_1(si.geom);
    auto anisotropic = m_anisotropic->eval_1(si.geom);
    auto sheen = m_sheen->eval_1(si.geom);
    auto sheen_tint = m_sheen_tint->eval_1(si.geom);
    auto clearcoat = m_clearcoat->eval_1(si.geom);
    auto clearcoat_gloss = m_clearcoat_gloss->eval_1(si.geom);
    // Precomputes
    auto c_dlin = mon2lin(base_color);
    auto c_dlum = .3 * c_dlin[0] + .6 * c_dlin[1] + .1 * c_dlin[2];
    auto c_tint = c_dlum > 0.f ? c_dlin / c_dlum : Color3(1);
    auto c_spec = lerp(metallic, specular * .8f * lerp(specular_tint, 1.f, c_tint), c_dlin);
    // Computes
    auto h = math::normalize(si.wi + wo);
    auto cos_theta_d = math::dot(si.wi, h);
    // Diffuse
    Float f_d90 = 0.5f + 2.f * math::sqr(cos_theta_d) * roughness;
    auto pow5 = [&](Float x) -> Float { return x * x * x * x * x; };
    Color3 f_diffuse = base_color / math::Pi<Float> * (1.f + (f_d90 - 1) * pow5(1 - cos_theta_i)) * (1.f + (f_d90 - 1) * pow5(1 - cos_theta_o));
    // Subsurface
    auto f_ss90 = math::sqr(cos_theta_d) * roughness;
    auto f_ss = (1.f + (f_ss90 - 1) * pow5(1 - cos_theta_i)) * (1.f + (f_ss90 - 1) * pow5(1 - cos_theta_o));
    Color3 f_subsurface = base_color * 1.25f / math::Pi<Float> * (f_ss * (1.f / (cos_theta_i + cos_theta_o) - 0.5f) + 0.5f);
    // Specular
    Float alpha = math::safe_sqrt(1.f - 0.9f * anisotropic);
    Float a_x = math::sqr(roughness) / alpha, a_y = math::sqr(roughness) * alpha;
    MicrofacetDistribution distr(MicrofacetDistribution::Type::GGX, a_x, a_y);
    Float Ds = distr.eval(h);
    Float Gs = distr.G(si.wi, wo, h);
    auto Fs = lerp(c_spec, schlick_fresnel(cos_theta_d), 1.f);
    Color3 f_specular = Fs * Ds * Gs / (4.f * cos_theta_i * cos_theta_o);
    // Sheen
    Color3 f_sheen = lerp(sheen_tint, 1.f, c_tint) * sheen * schlick_fresnel(cos_theta_d);
    // Clearcoat
    Float Fc = 0.04 + 0.96 * schlick_fresnel(cos_theta_d);
    Float Gc = smith_g1(cos_theta_i, 0.25) * smith_g1(cos_theta_o, 0.25);
    Float Dc = gtr1(Frame::cos_theta(h), lerp(clearcoat_gloss, .1f, .001f));
    Color3 f_clearcoat = clearcoat * Fc * Gc * Dc / (4.f * cos_theta_i * cos_theta_o);
    return f_diffuse + f_specular + f_clearcoat + f_sheen;
  }

  Float pdf(const BSDFContext &ctx,
            const SceneInteraction &si,
            const Vector3 &wo) const override {
  }

  MSK_DECL_COMP(BSDF)
 private:
  Float schlick_fresnel(float u) const {
    auto pow5 = [&](Float x) -> Float { return x * x * x * x * x; };
    return pow5(math::clamp(1.f - u, 0.f, 1.f));
  }

  Float smith_g1(Float NdotV, Float alphaG) const {
    Float a = alphaG * alphaG;
    Float b = NdotV * NdotV;
    return 1 / (NdotV + math::safe_sqrt(a + b - a * b));
  }

  Float gtr1(Float cos_theta_h, Float a) const {
    Float a2 = a * a;
    Float t = 1 + (a2 - 1) * cos_theta_h * cos_theta_h;
    return (a2 - 1) / (math::Pi<Float> * std::log(a2) * t);
  }

  Color3 lerp(Color3 t, Color3 v1, Color3 v2) const {
    return (1 - t) * v1 + t * v2;
  }

  Float lerp(Float t, Float v1, Float v2) const {
    return (1 - t) * v1 + t * v2;
  }

  Color3 mon2lin(const Color3 &x) const {
    return Color3(std::pow(x[0], 2.2), std::pow(x[1], 2.2), std::pow(x[2], 2.2));
  }

  std::shared_ptr<Texture> m_base_color, m_subsurface, m_metallic, m_specular, m_specular_tint;
  std::shared_ptr<Texture> m_roughness, m_anisotropic, m_sheen, m_sheen_tint,
      m_clearcoat, m_clearcoat_gloss;
};

MSK_EXPORT_PLUGIN(DisneyPrincipledBRDF)

}  // namespace misaki::render