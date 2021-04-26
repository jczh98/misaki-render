#pragma once

#include <aspirin/interaction.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>
#include <aspirin/volume.h>

namespace aspirin {

class ConstVolume final : public Volume {
public:
    ConstVolume(const Properties &props) : Volume(props) {
        m_color = props.texture<Texture>("color", 1.f);
    }

    Spectrum eval(const Interaction &it) const override {
        return eval_impl(it);
    }

    APR_INLINE Spectrum eval_impl(const Interaction &it) const {
        SurfaceInteraction si;
        si.uv       = Vector2(0.f, 0.f);
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

    APR_DECLARE_CLASS()
protected:
    ref<Texture> m_color;
};

APR_IMPLEMENT_CLASS(ConstVolume, Volume)
APR_INTERNAL_PLUGIN(ConstVolume, "constvolume")

} // namespace aspirin