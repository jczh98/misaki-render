#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/interaction.h>
#include <misaki/render/srgb.h>
#include <misaki/render/texture.h>

namespace misaki {

class MSK_EXPORT UniformSpectrum : public Texture {
public:
    UniformSpectrum(const Properties &props)
        : Texture(props) {
        m_value = props.float_("value");
    }

    Spectrum eval(const SceneInteraction &si) const override {
        if ((si.wavelengths >= MSK_WAVELENGTH_MIN)
            .all() && (si.wavelengths <= MSK_WAVELENGTH_MAX).all()) {
            return Spectrum::Constant(m_value);
        } else {
            return Spectrum::Zero();
        }
    }

    float mean() const override { return m_value; }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "UniformSpectrum[" << std::endl
            << "  value = " << m_value << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    float m_value;
};

MSK_IMPLEMENT_CLASS(UniformSpectrum, Texture)
MSK_REGISTER_INSTANCE(UniformSpectrum, "uniform")

} // namespace misaki
