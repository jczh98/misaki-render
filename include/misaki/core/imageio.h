#pragma once

#include <OpenImageIO/imageio.h>

#include "fwd.h"

namespace misaki {

extern MSK_EXPORT void
write_float_rgb_image(const std::string &filename,
                      const Array<Color<float, 3>, 2> &bitmap);

extern MSK_EXPORT Array<Color<float, 3>, 2>
read_float_rgb_image(const std::string &filename);

} // namespace misaki