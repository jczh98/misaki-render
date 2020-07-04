#include <misaki/render/bsdf.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/texture.h>
#include <misaki/render/warp.h>

namespace misaki::render {

class DiffuseBSDF final : public BSDF {
 public:
  DiffuseBSDF(const Properties &props) : BSDF(props) {
    m_reflectance = props.texture<Texture>("reflectance", 0.5f);
    m_flags = +BSDFFlags::DiffuseReflection;
    m_components.push_back(m_flags);
  }

  std::pair<BSDFSample, Color3> sample(const BSDFContext &ctx,
                                       const PointGeometry &geom,
                                       const Vector3 &wi,
                                       const Vector2 &sample) const override {
    Float cos_theta_i = Frame::cos_theta(wi);
    BSDFSample bs;
    if (cos_theta_i <= 0.f || !ctx.is_enabled(BSDFFlags::DiffuseReflection)) return {bs, 0.f};
    bs.wo = warp::square_to_cosine_hemisphere(sample);
    bs.pdf = warp::square_to_cosine_hemisphere_pdf(bs.wo);
    bs.eta = 1.f;
    bs.sampled_type = +BSDFFlags::DiffuseReflection;
    bs.sampled_component = 0;
    return {bs, bs.pdf > 0.f ? m_reflectance->eval_3(geom) : 0.f};
  }

  Color3 eval(const BSDFContext &ctx,
              const PointGeometry &geom,
              const Vector3 &wi,
              const Vector3 &wo) const override {
    if (!ctx.is_enabled(BSDFFlags::DiffuseReflection))
      return 0.f;

    Float cos_theta_i = Frame::cos_theta(wi),
          cos_theta_o = Frame::cos_theta(wo);
    if (cos_theta_i > 0.f && cos_theta_o > 0.f) {
      return m_reflectance->eval_3(geom) * math::InvPi<Float> * cos_theta_o;
    } else {
      return Color3(0.f);
    }
  }

  Float pdf(const BSDFContext &ctx,
            const PointGeometry &geom,
            const Vector3 &wi,
            const Vector3 &wo) const override {
    if (!ctx.is_enabled(BSDFFlags::DiffuseReflection)) return 0.f;
    if (Frame::cos_theta(wi) > 0.f && Frame::cos_theta(wo) > 0.f) {
      return warp::square_to_cosine_hemisphere_pdf(wo);
    } else {
      return 0.f;
    }
  }

  MSK_DECL_COMP(BSDF)
 protected:
  std::shared_ptr<Texture> m_reflectance;
};

MSK_EXPORT_PLUGIN(DiffuseBSDF)

}  // namespace misaki::render