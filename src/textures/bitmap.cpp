#include <misaki/render/imageio.h>
#include <misaki/render/interaction.h>
#include <misaki/render/properties.h>
#include <misaki/render/texture.h>

namespace misaki::render {

class BitmapTexture final : public Texture {
 public:
  BitmapTexture(const Properties &props) : Texture(props) {
    m_transform = props.transform("to_uv", Transform4()).extract();
    auto filename = props.string("filename");
    try {
      m_bitmap = read_float_rgb_image(get_file_resolver()->resolve(filename).string());
    } catch (std::exception &e) {
      Throw("bitmap: {}", e.what());
    }
  }

  Float eval_1(const PointGeometry &geom) const override {
    auto uv = m_transform.transform_affine_point(geom.uv);
    uv = uv - Vector2(math::floor2int(uv));
    return texture::linear_sample2d(uv, m_bitmap).luminance();
  }

  Color3 eval_3(const PointGeometry &geom) const override {
    auto uv = m_transform.transform_affine_point(geom.uv);
    uv = uv - Vector2(math::floor2int(uv));
    auto to_linear = [&](const Color3 &v) -> Color3 {
      Color3 result;
      for (int i = 0; i < 3; ++i) {
        float value = v.coeff(i);
        if (value <= 0.04045f)
          result[i] = value * (1.0f / 12.92f);
        else
          result[i] = std::pow((value + 0.055f) * (1.0f / 1.055f), 2.4f);
      }
      return result;
    };
    return to_linear(texture::linear_sample2d(uv, m_bitmap));
  }

  MSK_DECL_COMP(Texture)
 private:
  math::Tensor<Color3, 2> m_bitmap;
  Transform3 m_transform;
};

MSK_EXPORT_PLUGIN(BitmapTexture)

}  // namespace misaki::render