#include <misaki/render/imageio.h>

namespace misaki::render {

void write_float_rgb_image(const std::string &filename, const math::Tensor<Color3, 2> &bitmap) {
  std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);
  OIIO::ImageSpec image_spec(bitmap.shape()[1], bitmap.shape()[0], 3, OIIO::TypeDesc::FLOAT);
  out->open(filename, image_spec);
  out->write_image(OIIO::TypeDesc::FLOAT, bitmap.raw_data());
  out->close();
}

}  // namespace misaki::render