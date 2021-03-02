#pragma once

#include "fwd.h"

namespace aspirin {

namespace detail {

#define APR_EXTERN_CLASS(Name)                                                 \
    extern template class APR_EXPORT Name<float, Color<float, 3>>;

#define APR_INSTANTIATE_CLASS(Name)                                            \
    template class APR_EXPORT Name<float, Color<float, 3>>;

template <typename Float_, typename Spectrum_>
constexpr const char *get_variant() {
    if constexpr (std::is_same_v<Float_, float> &&
                  std::is_same_v<Spectrum_, Color<float, 3>>)
        return "scalar_rgb";
    else
        return "";
}

} // namespace detail

} // namespace aspirin