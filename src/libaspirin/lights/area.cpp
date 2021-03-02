#include <aspirin/emitter.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/texture.h>

namespace aspirin {

class AreaLight final : public Light {
 public:
  AreaLight(const Properties &props) : Light(props) {
    m_radiance = props.texture<Texture>("radiance", 1.f);

    m_flags = +LightFlags::Surface;
  }

  std::pair<DirectSample, Color3>
  sample_direct(const PointGeometry &geom_ref, const Vector2 &position_sample) const override {
    DirectSample ds;
    std::tie(ds.geom, ds.pdf) = m_shape->sample_position(position_sample);
    ds.d = ds.geom.p - geom_ref.p;
    // Convert pdf to solid angle measure
    Float dist_squared = math::squared_norm(ds.d);
    ds.dist = std::sqrt(dist_squared);
    ds.d /= ds.dist;
    auto w_dot_n = math::dot(ds.d, ds.geom.shading.n);
    Float dp = std::abs(w_dot_n);
    if (dp != 0.f && w_dot_n < 0.f) {
      ds.pdf *= dist_squared / dp;
      Color3 emitted = m_radiance->eval_3(ds.geom) / ds.pdf;
      return {ds, emitted};
    } else {
      ds.pdf = 0.f;
      return {ds, 0.f};
    }
  }

  Float pdf_direct(const PointGeometry &geom_ref, const DirectSample &ds) const {
    Float pdf = m_shape->pdf_position(geom_ref),
          dp = std::abs(math::dot(ds.d, ds.geom.shading.n));
    pdf *= dp != 0 ? ds.dist * ds.dist / dp : 0.f;
    return pdf;
  }

  Color3 eval(const SceneInteraction &si) const override {
    return Frame::cos_theta(si.wi) > 0.f ? m_radiance->eval_3(si.geom) : 0.f;
  }

  void set_shape(Shape *shape) override {
    Light::set_shape(shape);
  }

  MSK_DECL_COMP(Light)
 private:
  std::shared_ptr<Texture> m_radiance;
};

MSK_EXPORT_PLUGIN(AreaLight)

}  // namespace aspirin