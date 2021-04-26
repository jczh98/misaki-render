#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

class SRGBTexture final : public Texture {
public:
    SRGBTexture(const Properties &props) : Texture(props) {
        m_value = props.color("color");
    }

    Float eval_1(const SurfaceInteraction &si) const override {
        return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
    }

    Color3 eval_3(const SurfaceInteraction &si) const override {
        return m_value;
    }

    Float mean() const override {
        return (m_value.x() + m_value.y() + m_value.z()) / 3.f;
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SRGBTexture[" << std::endl
            << "  value = " << m_value << std::endl
            << "]";
        return oss.str();
    }

    APR_DECLARE_CLASS()
private:
    Color3 m_value;
};

APR_IMPLEMENT_CLASS(SRGBTexture, Texture)
APR_INTERNAL_PLUGIN(SRGBTexture, "srgb")

} // namespace aspirin