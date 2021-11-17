#include <misaki/interaction.h>
#include <misaki/properties.h>
#include <misaki/manager.h>
#include <misaki/texture.h>

namespace misaki {

class CheckerboardTexture final : public Texture {
public:
    CheckerboardTexture(const Properties &props) : Texture(props) {
        m_color0    = props.texture("color0", .4f);
        m_color1    = props.texture("color1", .2f);
        m_transform = props.transform("to_uv", Transform4f()).extract();
    }
    float eval_1(const SurfaceInteraction &si) const override {
        const auto uv = m_transform.transform_affine_point(si.uv);
        const auto u  = uv.x() - std::floor(uv.x());
        const auto v  = uv.y() - std::floor(uv.y());
        if (u > .5f == v > .5f)
            return m_color0->eval_1(si);
        else
            return m_color1->eval_1(si);
    }

    Color3 eval_3(const SurfaceInteraction &si) const override {
        const auto uv = m_transform.transform_affine_point(si.uv);
        const auto u  = uv.x() - std::floor(uv.x());
        const auto v  = uv.y() - std::floor(uv.y());
        if (u > .5f == v > .5f)
            return m_color0->eval_3(si);
        else
            return m_color1->eval_3(si);
    }

    float mean() const override { return m_color0->mean() + m_color1->mean(); }

    MSK_DECLARE_CLASS()
private:
    ref<Texture> m_color0, m_color1;
    Transform3f m_transform;
};

MSK_IMPLEMENT_CLASS(CheckerboardTexture, Texture)
MSK_REGISTER_INSTANCE(CheckerboardTexture, "checkerboard")

} // namespace misaki