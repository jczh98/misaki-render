#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/core/manager.h>
#include <misaki/render/interaction.h>
#include <misaki/render/texture.h>
#include <misaki/render/srgb.h>

namespace misaki {

class MSK_EXPORT SRGBReflectanceSpectrum : public Texture {
public:
    SRGBReflectanceSpectrum(const Properties &props)
        : Texture(props) {
        Color3 color = props.color("color");

        m_value = srgb_model_fetch(color);
    }

    Spectrum eval(const SceneInteraction &si) const override {
        return srgb_model_eval(m_value, si.wavelengths);
    }

    float mean() const override {
        return srgb_model_mean(m_value);
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SRGBReflectanceSpectrum[" << std::endl
            << "  value = " << m_value << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    Color3 m_value;
};

MSK_IMPLEMENT_CLASS(SRGBReflectanceSpectrum, Texture)
MSK_REGISTER_INSTANCE(SRGBReflectanceSpectrum, "srgb")

}
