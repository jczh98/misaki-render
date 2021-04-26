#include <aspirin/interaction.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

class CheckerboardTexture final : public Texture {
public:
    CheckerboardTexture(const Properties &props) : Texture(props) {
        m_color0    = props.texture<Texture>("color0", .4f);
        m_color1    = props.texture<Texture>("color1", .2f);
        m_transform = props.transform("to_uv", Transform4()).extract();
    }
    Float eval_1(const SurfaceInteraction &si) const override {
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

    Float mean() const override { return m_color0->mean() + m_color1->mean(); }

    APR_DECLARE_CLASS()
private:
    ref<Texture> m_color0, m_color1;
    Transform3 m_transform;
};

APR_IMPLEMENT_CLASS(CheckerboardTexture, Texture)
APR_INTERNAL_PLUGIN(CheckerboardTexture, "checkerboard")

} // namespace aspirin