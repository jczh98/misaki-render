#include <aspirin/imageio.h>
#include <aspirin/interaction.h>
#include <aspirin/properties.h>
#include <aspirin/texture.h>

namespace aspirin {

class BitmapTexture final : public Texture {
public:
    BitmapTexture(const Properties &props) : Texture(props) {
        m_transform   = props.transform("to_uv", Transform4()).extract();
        auto filename = props.string("filename");
        try {
            Float mean   = 0.0;
            auto texture = read_float_rgb_image(
                get_file_resolver()->resolve(filename).string());
            m_bitmap = math::Tensor<Color3, 2>::from_linear_indexed(
                texture.shape(), [&](int i) {
                    auto ret = texture.raw_data()[i];
                    mean += ret.luminance();
                    return ret.to_linear();
                });
            m_mean = mean / math::hprod(m_bitmap.shape());
        } catch (std::exception &e) {
            Throw("bitmap: {}", e.what());
        }
    }

    Float eval_1(const PointGeometry &geom) const override {
        auto uv = m_transform.transform_affine_point(geom.uv);
        uv      = uv - Vector2(math::floor2int(uv));
        return texture::linear_sample2d(uv, m_bitmap).luminance();
    }

    Color3 eval_3(const PointGeometry &geom) const override {
        auto uv = m_transform.transform_affine_point(geom.uv);
        uv      = uv - Vector2(math::floor2int(uv));
        return texture::linear_sample2d(uv, m_bitmap);
    }

    Float mean() const override { return m_mean; }

    MSK_DECL_COMP(Texture)
private:
    math::Tensor<Color3, 2> m_bitmap;
    Transform3 m_transform;
    Float m_mean;
};

MSK_EXPORT_PLUGIN(BitmapTexture)

} // namespace aspirin