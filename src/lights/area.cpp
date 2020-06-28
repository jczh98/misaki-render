#include <misaki/render/light.h>
#include <misaki/render/properties.h>

namespace misaki::render {

class AreaLight final : public Light {
 public:
  AreaLight(const Properties &props) : Light(props) {
    m_radiance = props.color("radiance", 1.f);

    m_flags = +LightFlags::Surface;
  }

  Color3 eval(const PointGeometry &geom, const Vector3 &wi) const override {
    return Frame::cos_theta(wi) > 0.f ? m_radiance : 0.f;
  }

  void set_shape(Shape *shape) override {
    Light::set_shape(shape);
  }

  MSK_DECL_COMP(Light)
 private:
  Color3 m_radiance;
};

MSK_EXPORT_PLUGIN(AreaLight)

}  // namespace misaki::render