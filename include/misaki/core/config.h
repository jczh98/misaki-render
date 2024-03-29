#pragma once

#include "fwd.h"

namespace misaki {

namespace detail {

#define APR_EXTERN_CLASS(Name)                                                 \
    extern template class MSK_EXPORT Name<float, Color<float, 3>>;

#define APR_INSTANTIATE_CLASS(Name)                                            \
    template class MSK_EXPORT Name<float, Color<float, 3>>;

#define APR_INSTANTIATE_STRUCT(Name)                                           \
    template struct MSK_EXPORT Name<float, Color<float, 3>>;

#define APR_INVOKE_VARIANT(variant, func, ...)                                 \
    [&]() {                                                                    \
        if (variant == "scalar_rgb") {                                         \
            return func<float, Color<float, 3>>(__VA_ARGS__);                  \
        } else {                                                               \
            Throw("Unsupported variant: %s", variant);                         \
        }                                                                      \
    }()

template <typename Float_, typename Spectrum_>
constexpr const char *get_variant() {
    if constexpr (std::is_same_v<Float_, float> &&
                  std::is_same_v<Spectrum_, Color<float, 3>>)
        return "scalar_rgb";
    else
        return "";
}

} // namespace detail

} // namespace misaki