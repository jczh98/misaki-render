#pragma once

#include <OpenImageIO/imageio.h>

#include "fwd.h"

namespace misaki::render {

extern MSK_EXPORT void write_float_rgb_image(const std::string &filename, const math::Tensor<Color3, 2> &bitmap);

}