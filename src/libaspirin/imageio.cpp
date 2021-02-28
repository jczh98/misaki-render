#include <aspirin/imageio.h>
#include <aspirin/logger.h>

namespace aspirin {

void write_float_rgb_image(const std::string &filename, const Array<Color3, 2> &bitmap) {
  std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);
  OIIO::ImageSpec image_spec(bitmap.shape()[1], bitmap.shape()[0], 3, OIIO::TypeDesc::FLOAT);
  out->open(filename, image_spec);
  out->write_image(OIIO::TypeDesc::FLOAT, bitmap.raw_data());
  out->close();
}

Array<Color3, 2> read_float_rgb_image(const std::string &filename) {
  auto in = OIIO::ImageInput::open(filename);
  if (!in) {
    Throw("file {} not exists!", filename);
  }
  const OIIO::ImageSpec &spec = in->spec();
  int xres = spec.width;
  int yres = spec.height;
  int channels = spec.nchannels;
  Log(Info, "Loding image file \"{}\" ({}x{}, {} channels) ..", filename, xres, yres, channels);
  if (channels == 3) {
      Array<Color3, 2> bitmap({yres, xres});
    in->read_image(OIIO::TypeDesc::FLOAT, bitmap.raw_data());
    in->close();
    return bitmap;
  } else if (channels == 4) {
      Array<Color4, 2> bitmap({yres, xres});
    in->read_image(OIIO::TypeDesc::FLOAT, bitmap.raw_data());
    in->close();
    auto result = Array<Color3, 2>::from_linear_indexed(bitmap.shape(),
                                                               [&](int i) {
                                                                 const auto c4 = bitmap.raw_data()[i];
                                                                 return Color3(c4.r(), c4.g(), c4.b());
                                                               });
    return result;
  } else {
    Throw("Current file {} have channel of {} instead of 3", filename, channels);
  }
}

}  // namespace aspirin