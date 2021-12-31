#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
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

ref<Texture> Texture::D65(float scale) {
    Properties props("d65");
    props.set_float("scale", scale);
    ref<Texture> texture =
        InstanceManager::get()->create_instance<Texture>(props);
    std::vector<ref<Object>> children = texture->expand();
    if (!children.empty())
        return (Texture *) children[0].get();
    return texture;
}

MSK_IMPLEMENT_CLASS(Texture, Object, "texture")

} // namespace misaki