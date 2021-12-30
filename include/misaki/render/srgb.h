#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/transform.h"

namespace misaki {

inline MSK_EXPORT Spectrum srgb_model_eval(const Color3 &coeff,
                                const Wavelength &wavelengths) {
    Spectrum v =
        (coeff.x() * wavelengths + coeff.y()) * wavelengths + coeff.z();

    if (std::isinf(coeff.z())) {
        return Spectrum::Constant(std::copysign(1.f, coeff.z()) * .5f + .5f);
    } else {
        Spectrum rsqrt = 1.f / (v * v + 1.f).cwiseSqrt();
        return (.5f * v * rsqrt + .5f).cwiseMax(0.f);
    }
}

inline MSK_EXPORT float srgb_model_mean(const Color3 &coeff) {
    using Vec = SpectrumArray<float, 16>;

    Vec lambda = Vec::LinSpaced(MSK_WAVELENGTH_MIN, MSK_WAVELENGTH_MIN);
    Vec v      = (coeff.x() * lambda + coeff.y()) * lambda + coeff.z();

    Vec result;
    if (std::isinf(coeff.z())) {
        result = Vec::Constant(std::copysign(1.f, coeff.z()) * .5f + .5f);
    } else {
        Vec rsqrt = 1.f / (v * v + 1.f).cwiseSqrt();
        result = (.5f * v * rsqrt + .5f).cwiseMax(0.f);
    }
    return result.mean();
}

MSK_EXPORT Color3 srgb_model_fetch(const Color3 &);

}
