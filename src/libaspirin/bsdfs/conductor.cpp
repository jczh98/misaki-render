#include <aspirin/bsdf.h>
#include <aspirin/fresnel.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>
#include <aspirin/warp.h>

namespace aspirin {

class ConductorBSDF final : public BSDF {
 public:
  ConductorBSDF(const Properties &props) : BSDF(props) {
    m_specular_reflectance = props.texture<Texture>("specular_reflectance", 1.f);
    // Initially set up for mirror
    m_eta = props.texture<Texture>("eta", 0.f);
    m_k = props.texture<Texture>("k", 1.f);
    m_flags = +BSDFFlags::DeltaReflection;
    m_components.push_back(m_flags);
  }

  std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                       const SceneInteraction &si,
                                       Float sample1,
                                       const Vector2 &sample) const override {
    Float cos_theta_i = Frame::cos_theta(si.wi);
    BSDFSample bs;
    if (cos_theta_i <= 0.f || !ctx.is_enabled(BSDFFlags::DeltaReflection)) return {bs, 0.f};
    bs.wo = reflect(si.wi);
    bs.pdf = 1.f;
    bs.eta = 1.f;
    bs.sampled_type = +BSDFFlags::DeltaReflection;
    bs.sampled_component = 0;
    Color3 value = m_specular_reflectance->eval_3(si.geom) * fresnel_conductor(cos_theta_i,
                                                                               m_eta->eval_3(si.geom),
                                                                               m_k->eval_3(si.geom));
    return {bs, value};
  }

  Color3 eval(const BSDFContext &ctx,
              const SceneInteraction &si,
              const Vector3 &wo) const override {
    return 0.f;
  }

  Float pdf(const BSDFContext &ctx,
            const SceneInteraction &si,
            const Vector3 &wo) const override {
    return 0.f;
  }

  MSK_DECL_COMP(BSDF)
 protected:
  std::shared_ptr<Texture> m_specular_reflectance, m_eta, m_k;
};

MSK_EXPORT_PLUGIN(ConductorBSDF)

}  // namespace aspirin