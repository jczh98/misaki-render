#include <misaki/render/interaction.h>
#include <misaki/render/properties.h>
#include <misaki/render/texture.h>

namespace misaki::render {

class CheckerboardTexture final : public Texture {
 public:
  CheckerboardTexture(const Properties &props) : Texture(props) {
    m_color0 = props.texture<Texture>("color0", .4f);
    m_color1 = props.texture<Texture>("color1", .2f);
  }
  Float eval_1(const PointGeometry &geom) const override {
    const auto u = geom.uv.x() - std::floor(geom.uv.x());
    const auto v = geom.uv.y() - std::floor(geom.uv.y());
    if (u > .5f == v > .5f)
      return m_color0->eval_1(geom);
    else
      return m_color1->eval_1(geom);
  }

  Color3 eval_3(const PointGeometry &geom) const override {
    const auto u = geom.uv.x() - std::floor(geom.uv.x());
    const auto v = geom.uv.y() - std::floor(geom.uv.y());
    if (u > .5f == v > .5f)
      return m_color0->eval_3(geom);
    else
      return m_color1->eval_3(geom);
  }

  MSK_DECL_COMP(Texture)
 private:
  std::shared_ptr<Texture> m_color0, m_color1;
};

MSK_EXPORT_PLUGIN(CheckerboardTexture)

}  // namespace misaki::render