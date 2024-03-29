#pragma once

#include <misaki/render/interaction.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
#include <misaki/render/texture.h>
#include <misaki/render/volume.h>

namespace misaki {

class ConstVolume final : public Volume {
public:
    ConstVolume(const Properties &props) : Volume(props) {
        m_color = props.texture("color", 1.f);
    }

    Spectrum eval(const Interaction &it) const override {
        return eval_impl(it);
    }

    MSK_INLINE Spectrum eval_impl(const Interaction &it) const {
        SurfaceInteraction si;
        si.uv       = Eigen::Vector2f(0.f, 0.f);
        auto result = m_color->eval_3(si);
        return result;
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "ConstVolume[" << std::endl
            << "  world_to_local = " << m_world_to_local << "," << std::endl
            << "  color = " << m_color->to_string() << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
protected:
    ref<Texture> m_color;
};

MSK_IMPLEMENT_CLASS(ConstVolume, Volume)
MSK_REGISTER_INSTANCE(ConstVolume, "constvolume")

} // namespace misaki