#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/interaction.h>
#include <misaki/render/srgb.h>
#include <misaki/render/texture.h>

namespace misaki {

class MSK_EXPORT SRGBEmitterSpectrum : public Texture {
public:
    SRGBEmitterSpectrum(const Properties &props)
        : Texture(props) {
        Color3 color = props.color("color");

        float scale = color.maxCoeff() * 2.f;

        if (scale != 0.f)
            color /= scale;

        m_value = srgb_model_fetch(color);

        Properties props2("d65");
        props2.set_float("scale", props.float_("scale", 1.f) * scale);
        InstanceManager *imgr = InstanceManager::get();
        m_d65 = (Texture *) imgr->create_instance<Texture>(props2)
                                ->expand()
                                .at(0)
                                .get();
    }

    Spectrum eval(const SceneInteraction &si) const override {
        return m_d65->eval(si)  * srgb_model_eval(m_value, si.wavelengths);
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SRGBEmitterSpectrum[" << std::endl
            << "  value = " << m_value << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    Color3 m_value;
    ref<Texture> m_d65;
};

MSK_IMPLEMENT_CLASS(SRGBEmitterSpectrum, Texture)
MSK_REGISTER_INSTANCE(SRGBEmitterSpectrum, "srgb_d65")

} // namespace misaki
