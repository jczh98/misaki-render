#pragma once

#include "fwd.h"

namespace aspirin {

namespace detail {

template <typename Float_, typename Spectrum_> constexpr const char *get_variant() {
    if constexpr (std::is_same_v<Float_, float> &&
                                         std::is_same_v<Spectrum_, Color<float, 3>>)
        return "scalar_rgb";
    else
    return "";
}

}

}