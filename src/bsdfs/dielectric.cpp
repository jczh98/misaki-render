#include <misaki/render/bsdf.h>
#include <misaki/render/fresnel.h>
#include <misaki/render/properties.h>

namespace misaki::render {

class DielectricBSDF final : public BSDF {
 public:
  DielectricBSDF(const Properties &props) : BSDF(props) {
    Float int_ior = props.get_float("int_ior", 1.49);
    Float ext_ior = props.get_float("ext_ior", 1.00028);
    m_eta = int_ior / ext_ior;
    m_specular_reflectance = props.color("specular_reflectance", 1.f);
    m_specular_transmittance = props.color("specular_transmittance", 1.f);
    m_components.push_back(+BSDFFlags::DeltaReflection);
    m_components.push_back(+BSDFFlags::DeltaTransmission);
    m_flags = m_components[0] | m_components[1];
  }

  std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                       const PointGeometry &geom,
                                       const Vector3 &wi,
                                       const Vector2 &sample) const override {
    bool has_reflection = ctx.is_enabled(BSDFFlags::DeltaReflection, 0),
         has_transmission = ctx.is_enabled(BSDFFlags::DeltaTransmission, 1);
    Float cos_theta_i = Frame::cos_theta(wi);
    auto [r_i, cos_theta_t, eta_it, eta_ti] = fresnel(cos_theta_i, m_eta);
    Float t_i = 1.f - r_i;
    BSDFSample bs;
    bool selected_r;
    if (has_reflection && has_transmission) {
      selected_r = sample.x() <= r_i;
      bs.pdf = selected_r ? r_i : t_i;
    } else {
      if (has_reflection || has_transmission) {
        selected_r = has_reflection;
        bs.pdf = 1.f;
      } else {
        return {bs, 0.f};
      }
    }
    bs.sampled_component = selected_r ? 0 : 1;
    bs.sampled_type = selected_r ? +BSDFFlags::DeltaReflection : +BSDFFlags::DeltaTransmission;
    bs.wo = selected_r ? reflect(wi) : refract(wi, cos_theta_t, eta_ti);
    bs.eta = selected_r ? 1.f : eta_it;
    Color3 reflectance = 1.f, transmittance = 1.f;
    if (selected_r) reflectance = m_specular_reflectance;
    if (!selected_r) transmittance = m_specular_transmittance;
    Color3 weight;
    if (has_reflection && has_transmission)
      weight = 1.f;
    else if (has_reflection || has_transmission) {
      weight = has_reflection ? r_i : t_i;
    }
    if (selected_r) weight *= reflectance;
    if (!selected_r) {
      Float factor = (ctx.mode == TransportMode::Radiance) ? eta_ti : 1.f;
      weight *= transmittance * factor * factor;
    }
    return {bs, weight};
  }

  Color3 eval(const BSDFContext &ctx,
              const PointGeometry &geom,
              const Vector3 &wi,
              const Vector3 &wo) const override {
    return 0.f;
  }

  Float pdf(const BSDFContext &ctx,
            const PointGeometry &geom,
            const Vector3 &wi,
            const Vector3 &wo) const override {
    return 0;
  }

  MSK_DECL_COMP(BSDF)
 private:
  Float m_eta;
  Color3 m_specular_reflectance, m_specular_transmittance;
};

MSK_EXPORT_PLUGIN(DielectricBSDF)

}  // namespace misaki::render