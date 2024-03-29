#include <misaki/properties.h>
#include <misaki/texture.h>

namespace misaki {

class SRGBTexture final : public Texture {
public:
    SRGBTexture(const Properties &props) : Texture(props) {
        m_value = props.color("color");
    }

    float eval_1(const SurfaceInteraction &si) const override {
        return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
    }

    Color3 eval_3(const SurfaceInteraction &si) const override {
        return m_value;
    }

    float mean() const override {
        return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SRGBTexture[" << std::endl
            << "  value = " << m_value << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    Color3 m_value;
};

MSK_IMPLEMENT_CLASS(SRGBTexture, Texture)
MSK_INTERNAL_PLUGIN(SRGBTexture, "srgb")

} // namespace misaki