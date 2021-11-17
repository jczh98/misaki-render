#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/texture.h>

namespace misaki {

Texture::Texture(const Properties &props) : m_id(props.id()) {}

Texture::~Texture() {}

float Texture::eval_1(const SurfaceInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_1");
}

Color3 Texture::eval_3(const SurfaceInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_3");
}

float Texture::mean() const { MSK_NOT_IMPLEMENTED("mean"); }

MSK_IMPLEMENT_CLASS(Texture, Object, "texture")
MSK_IMPLEMENT_CLASS(ConstantSpectrumTexture, Texture)

} // namespace misaki