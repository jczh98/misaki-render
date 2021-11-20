#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/interaction.h>
#include <misaki/render/texture.h>

namespace misaki {

Texture::Texture(const Properties &props) : m_id(props.id()) {}

Texture::~Texture() {}

float Texture::eval_1(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_1");
}

Color3 Texture::eval_3(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_3");
}

float Texture::mean() const { MSK_NOT_IMPLEMENTED("mean"); }

float ConstantSpectrumTexture::eval_1(const SceneInteraction &si) const {
    return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
}

Color3 ConstantSpectrumTexture::eval_3(const SceneInteraction &si) const {
    return m_value;
}

float ConstantSpectrumTexture::mean() const {
    return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
}

MSK_IMPLEMENT_CLASS(Texture, Object, "texture")
MSK_IMPLEMENT_CLASS(ConstantSpectrumTexture, Texture)

} // namespace misaki