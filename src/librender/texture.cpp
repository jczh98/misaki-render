#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/interaction.h>
#include <misaki/render/texture.h>
#include <misaki/render/srgb.h>

namespace misaki {

Texture::Texture(const Properties &props) : m_id(props.id()) {}

Texture::~Texture() {}

float Texture::eval_1(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_1");
}

Spectrum Texture::eval(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval");
}

Color3 Texture::eval_3(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_3");
}

float Texture::mean() const { MSK_NOT_IMPLEMENTED("mean"); }

ConstantSpectrumTexture::ConstantSpectrumTexture(const Color3 &value)
    : Texture(Properties()) {
    m_value = srgb_model_fetch(value);
}

float ConstantSpectrumTexture::eval_1(const SceneInteraction &si) const {
    return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
}

Spectrum ConstantSpectrumTexture::eval(const SceneInteraction &si) const {
    return srgb_model_eval(m_value, si.wavelengths);
}

Color3 ConstantSpectrumTexture::eval_3(const SceneInteraction &si) const {
    MSK_NOT_IMPLEMENTED("eval_3");
}

float ConstantSpectrumTexture::mean() const {
    return srgb_model_mean(m_value);
}

MSK_IMPLEMENT_CLASS(Texture, Object, "texture")
MSK_IMPLEMENT_CLASS(ConstantSpectrumTexture, Texture)

} // namespace misaki