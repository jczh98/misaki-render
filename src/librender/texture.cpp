#include <misaki/logger.h>
#include <misaki/properties.h>
#include <misaki/texture.h>

namespace misaki {

Texture::Texture(const Properties &props) : m_id(props.id()) {}

Texture::~Texture() {}

Float Texture::eval_1(const SurfaceInteraction &si) const {
    APR_NOT_IMPLEMENTED("eval_1");
}

Color3 Texture::eval_3(const SurfaceInteraction &si) const {
    APR_NOT_IMPLEMENTED("eval_3");
}

Float Texture::mean() const { APR_NOT_IMPLEMENTED("mean"); }

APR_IMPLEMENT_CLASS(Texture, Object, "texture")

} // namespace misaki