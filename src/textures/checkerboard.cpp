#include <misaki/render/interaction.h>
#include <misaki/render/properties.h>
#include <misaki/render/texture.h>

namespace misaki::render {

class CheckerboardTexture final : public Texture {
 public:
  CheckerboardTexture(const Properties &props) : Texture(props) {
    m_color0 = props.texture<Texture>("color0", .4f);
    m_color1 = props.texture<Texture>("color1", .2f);
		m_transform = props.transform("to_uv", Transform4()).extract();
  }
  Float eval_1(const PointGeometry &geom) const override {
    const auto uv = m_transform.transform_affine_point(geom.uv);
    const auto u = uv.x() - std::floor(uv.x());
    const auto v = uv.y() - std::floor(uv.y());
    if (u > .5f == v > .5f)
      return m_color0->eval_1(geom);
    else
      return m_color1->eval_1(geom);
  }

  Color3 eval_3(const PointGeometry &geom) const override {
		const auto uv = m_transform.transform_affine_point(geom.uv);
    const auto u = uv.x() - std::floor(uv.x());
    const auto v = uv.y() - std::floor(uv.y());
    if (u > .5f == v > .5f)
      return m_color0->eval_3(geom);
    else
      return m_color1->eval_3(geom);
  }

  MSK_DECL_COMP(Texture)
 private:
  std::shared_ptr<Texture> m_color0, m_color1;
	Transform3 m_transform;
};

MSK_EXPORT_PLUGIN(CheckerboardTexture)

}  // namespace misaki::render