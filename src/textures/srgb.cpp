#include <misaki/render/properties.h>
#include <misaki/render/texture.h>

namespace misaki::render {

class SRGBTexture final : public Texture {
 public:
  SRGBTexture(const Properties &props) : Texture(props) {
    m_value = props.color("color");
  }

  Float eval_1(const PointGeometry &geom) const override {
    return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
  }

  Color3 eval_3(const PointGeometry &geom) const override {
    return m_value;
  }

  MSK_DECL_COMP(Texture)
 private:
  Color3 m_value;
};

MSK_EXPORT_PLUGIN(SRGBTexture)
}  // namespace misaki::render