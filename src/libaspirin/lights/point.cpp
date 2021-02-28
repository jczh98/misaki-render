#include <aspirin/light.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/texture.h>

namespace aspirin {

class PointLight final : public Light {
 public:
  PointLight(const Properties &props) : Light(props) {
    if (props.has_property("position")) {
      if (props.has_property("to_world"))
        Throw(
            "Only one of the parameters 'position' and 'to_world' "
            "can be specified at the same time!'");
      m_world_transform = Transform4f::translate(props.vector3("position"));
    }
    m_intensity = props.texture<Texture>("intensity", 1.f);
    m_flags = +LightFlags::DeltaPosition;
  }

  std::pair<DirectSample, Color3>
  sample_direct(const PointGeometry &geom_ref, const Vector2 &position_sample) const override {
    DirectSample ds;
    ds.geom = PointGeometry::make_degenerated(m_world_transform.translation());
    ds.pdf = 1.f;
    ds.d = ds.geom.p - geom_ref.p;
    ds.dist = math::norm(ds.d);
    Float inv_dist = 1.f / ds.dist;
    ds.d *= inv_dist;
    return {ds, m_intensity->eval_3(ds.geom) * inv_dist * inv_dist};
  }

  Float pdf_direct(const PointGeometry &geom_ref, const DirectSample &ds) const {
    return 0.f;
  }

  Color3 eval(const SceneInteraction &si) const override {
    return 0.f;
  }

  MSK_DECL_COMP(Light)
 private:
  std::shared_ptr<Texture> m_intensity;
};

MSK_EXPORT_PLUGIN(PointLight)

}  // namespace aspirin