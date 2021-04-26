#pragma once

#include <OpenImageIO/imageio.h>

#include "fwd.h"

namespace aspirin {

extern APR_EXPORT void
write_float_rgb_image(const std::string &filename,
                      const Array<Color<float, 3>, 2> &bitmap);

extern APR_EXPORT Array<Color<float, 3>, 2>
read_float_rgb_image(const std::string &filename);

} // namespace aspirin