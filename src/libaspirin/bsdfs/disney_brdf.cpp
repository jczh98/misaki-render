#include <aspirin/bsdf.h>
#include <aspirin/logger.h>
#include <aspirin/microfacet.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>
#include <aspirin/warp.h>

namespace aspirin {

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
    BSDFSample bs;
    Float cos_theta_i = Frame::cos_theta(si.wi);
    bool has_specular = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
         has_diffuse = ctx.is_enabled(BSDFFlags::DiffuseReflection, 1);
    if ((!has_specular && !has_diffuse) || cos_theta_i <= 0) return {bs, 0.f};
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
    Float prob_diffuse = (1 - metallic) / 2;
    if (sample1 < prob_diffuse) {
      bs.wo = sample_diffuse(sample);
      bs.sampled_component = 1;
      bs.sampled_type = +BSDFFlags::DiffuseReflection;
    } else {
      Float alpha = math::safe_sqrt(1.f - 0.9f * anisotropic);
      Float a_x = std::max(.001f, math::sqr(roughness) / alpha),
            a_y = std::max(.001f, math::sqr(roughness) * alpha);
      Float gtr2 = 1 / (1 + clearcoat);
      if (sample1 - prob_diffuse < gtr2) {
        bs.wo = sample_specular(si.wi, sample, a_x, a_y);
      } else {
        bs.wo = sample_clearcoat(si.wi, sample, roughness);
      }
      bs.sampled_component = 0;
      bs.sampled_type = +BSDFFlags::GlossyReflection;
      bs.eta = 1.f;
    }
    Color3 result(0.f);
    bs.pdf = pdf(ctx, si, bs.wo);
    if (bs.pdf <= 0.f) return {bs, 0.f};
    result = eval(ctx, si, bs.wo);
    return {bs, result / bs.pdf};
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
    auto cos_theta_d = math::dot(wo, h);
    auto cos_theta_h = Frame::cos_theta(h);
    auto sin_theta_h = Frame::sin_theta(h);
    auto tan_theta_i = Frame::tan_theta(si.wi);
    auto tan_theta_o = Frame::tan_theta(wo);
    auto [sin_phi_h, cos_phi_h] = Frame::sincos_phi(h);
    auto [sin_phi_i, cos_phi_i] = Frame::sincos_phi(si.wi);
    auto [sin_phi_o, cos_phi_o] = Frame::sincos_phi(wo);
    // Diffuse
    Float f_d90 = 0.5f + 2.f * math::sqr(cos_theta_d) * roughness;
    Float fl = schlick_weight(cos_theta_i), fv = schlick_weight(cos_theta_o);
    Float f_d = math::lerp(1.f, f_d90, fl) * math::lerp(1.f, f_d90, fv);
    // Subsurface
    Float f_ss90 = math::sqr(cos_theta_d) * roughness;
    Float f_ss = math::lerp(1.f, f_ss90, fl) * math::lerp(1.f, f_ss90, fv);
    Float f_subsurface = 1.25 * (f_ss * (1 / (cos_theta_i + cos_theta_o) - .5) + .5);
    // Specular
    Float alpha = math::safe_sqrt(1.f - 0.9f * anisotropic);
    Float a_x = std::max(.001f, math::sqr(roughness) / alpha),
          a_y = std::max(.001f, math::sqr(roughness) * alpha);
    Float Ds = microfacet::gtr2_aniso(sin_theta_h, cos_theta_h, sin_phi_h, cos_phi_h, a_x, a_y);
    Color3 Fs = lerp(c_spec, 1.f, schlick_weight(cos_theta_d));
    Float Gs = microfacet::smith_g1_ggx_aniso(tan_theta_i, sin_phi_i, cos_phi_i, a_x, a_y) * microfacet::smith_g1_ggx_aniso(tan_theta_o, sin_phi_o, cos_phi_o, a_x, a_y);
    Color3 f_specular = Fs * Ds * Gs;
    // Sheen
    Color3 f_sheen = lerp(1.f, c_tint, sheen_tint) * schlick_weight(cos_theta_d) * sheen;
    // Clearcoat
    Float Dc = microfacet::gtr1(cos_theta_h, math::lerp(.1f, .001f, clearcoat_gloss));
    Float Fc = math::lerp(.04f, 1.f, schlick_weight(cos_theta_d));
    Float Gc = microfacet::smith_g1_ggx(cos_theta_i, 0.25) * microfacet::smith_g1_ggx(cos_theta_o, 0.25);
    Color3 f_clearcoat = .25 * clearcoat * Gc * Fc * Dc;
    // Result
    Color3 f_diffuse = ((1 / math::Pi<Float>)*math::lerp(f_d, f_subsurface, subsurface) * c_dlin + f_sheen) * (1 - metallic);
    if (has_diffuse && has_specular)
      return cos_theta_o * (f_diffuse + f_specular + f_clearcoat);
    else if (has_diffuse)
      return f_diffuse * cos_theta_o;
    else
      return (f_specular + f_clearcoat) * cos_theta_o;
  }

  Float pdf(const BSDFContext &ctx,
            const SceneInteraction &si,
            const Vector3 &wo) const override {
    bool has_specular = ctx.is_enabled(BSDFFlags::GlossyReflection, 0),
         has_diffuse = ctx.is_enabled(BSDFFlags::DiffuseReflection, 1);
    Float cos_theta_i = Frame::cos_theta(si.wi),
          cos_theta_o = Frame::cos_theta(wo);
    auto m = math::normalize(wo + si.wi);
    if ((!has_specular && !has_diffuse) ||
        !(cos_theta_i > 0.f && cos_theta_o > 0.f && si.wi.dot(m) > 0.f && wo.dot(m) > 0.f)) {
      return 0.f;
    }
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
    // Computes
    auto h = math::normalize(si.wi + wo);
    auto cos_theta_d = math::dot(si.wi, h);
    auto cos_theta_h = Frame::cos_theta(h);
    auto sin_theta_h = Frame::sin_theta(h);
    auto tan_theta_i = Frame::tan_theta(si.wi);
    auto tan_theta_o = Frame::tan_theta(wo);
    auto [sin_phi_h, cos_phi_h] = Frame::sincos_phi(h);
    auto [sin_phi_i, cos_phi_i] = Frame::sincos_phi(si.wi);
    auto [sin_phi_o, cos_phi_o] = Frame::sincos_phi(wo);
    // Pdf
    Float pdf = 0.f;
    Float prob_diffuse = (1 - metallic) / 2.f;
    Float prob_specular = (1.f / (1 + clearcoat));
    Float prob_clearcoat = 1.f - prob_specular;
    Float alpha = math::safe_sqrt(1.f - 0.9f * anisotropic);
    Float a_x = std::max(.001f, math::sqr(roughness) / alpha),
          a_y = std::max(.001f, math::sqr(roughness) * alpha);
    const Float specular_d = microfacet::gtr2_aniso(
        sin_theta_h, cos_theta_h, sin_phi_h, cos_phi_h, a_x, a_y);
    auto pdf_diffuse = warp::square_to_cosine_hemisphere_pdf(wo);
    auto pdf_specular = microfacet::smith_g1_ggx_aniso(tan_theta_i, sin_phi_i, cos_phi_i, a_x, a_y) * specular_d / (4 * cos_theta_i);
    const Float clearcoat_d = microfacet::gtr1(cos_theta_h, clearcoat_gloss);
    auto pdf_clearcoat = cos_theta_h * clearcoat_d / (4 * cos_theta_d);
    if (has_diffuse && has_specular) {
      pdf = prob_diffuse * pdf_diffuse + (1.f - prob_diffuse) * (prob_specular * pdf_specular + prob_clearcoat * pdf_clearcoat);
    } else if (has_diffuse) {
      pdf = pdf_diffuse;
    } else {
      pdf = prob_specular * pdf_specular + prob_clearcoat * pdf_clearcoat;
    }
    return pdf;
  }

  MSK_DECL_COMP(BSDF)
 private:
  Vector3 sample_diffuse(const Vector2 &sample2) const {
    return warp::square_to_cosine_hemisphere(sample2);
  }

  Vector3 sample_specular(const Vector3 &wi, const Vector2 &sample, Float ax, Float ay) const {
    auto wh = microfacet::sample_gtr2_aniso(ax, ay, sample);
    auto wo = reflect(wi, wh);
    return wo;
  }

  Vector3 sample_clearcoat(const Vector3 &wi, const Vector2 &sample, Float roughness) const {
    auto wh = microfacet::sample_gtr1(roughness, sample);
    auto wo = reflect(wi, wh);
    return wo;
  }

  Color3 lerp(Color3 v1, Color3 v2, Color3 t) const {
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

}  // namespace aspirin