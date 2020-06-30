#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/texture.h>

namespace misaki::render {

Texture::Texture(const Properties &props) : m_id(props.id()) {
}

Float Texture::eval_1(const PointGeometry &geom) const {
  MSK_NOT_IMPLEMENTED("eval_1");
}

Color3 Texture::eval_3(const PointGeometry &geom) const {
  MSK_NOT_IMPLEMENTED("eval_3");
}

MSK_REGISTER_CLASS(Texture)

}  // namespace misaki::render