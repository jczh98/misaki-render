#pragma once

#include "fwd.h"

namespace misaki::warp {

template <typename Value> Value circ(Value x) {
    return math::safe_sqrt(-x * x + 1.f);
}

MSK_INLINE Eigen::Vector2f
square_to_uniform_triangle(const Eigen::Vector2f &sample) {
    float t = math::safe_sqrt(1.f - sample.x());
    return Eigen::Vector2f(1.f - t, t * sample.y());
}

MSK_INLINE Eigen::Vector2f
square_to_uniform_disk_concentric(const Eigen::Vector2f &sample) {
    float x = 2.f * sample.x() - 1.f;
    float y = 2.f * sample.y() - 1.f;
    float phi, r;
    if (x == 0 && y == 0) {
        r = phi = 0;
    } else if (x * x > y * y) {
        r   = x;
        phi = (math::Pi<float> / 4.f) * (y / x);
    } else {
        r   = y;
        phi = (math::Pi<float> / 2.f) - (x / y) * (math::Pi<float> / 4.f);
    }
    return Eigen::Vector2f(r * std::cos(phi), r * std::sin(phi));
}

MSK_INLINE Eigen::Vector3f
square_to_cosine_hemisphere(const Eigen::Vector2f &sample) {
    const Eigen::Vector2f p = square_to_uniform_disk_concentric(sample);
    float z                 = math::safe_sqrt(1.f - p.squaredNorm());
    return Eigen::Vector3f(p.x(), p.y(), z);
}

MSK_INLINE float square_to_cosine_hemisphere_pdf(const Eigen::Vector3f &v) {
    return math::InvPi<float> * v.z();
}

/// Uniformly sample a vector on the unit sphere with respect to solid angles
MSK_INLINE Eigen::Vector3f
square_to_uniform_sphere(const Eigen::Vector2f &sample) {
    float z = -2.f * sample.y() + 1.f, r = circ(z);
    float t = 2.f * math::Pi<float> * sample.x();
    float s = std::sin(t);
    float c = std::cos(t);
    return { r * c, r * s, z };
}

/// Density of \ref square_to_uniform_sphere() with respect to solid angles
template <bool TestDomain = false>
MSK_INLINE float square_to_uniform_sphere_pdf(const Eigen::Vector3f &v) {
    if constexpr (TestDomain)
        if (v.squaredNorm() - 1.f > math::RayEpsilon<float>) {
            return float(0);
        } else {
            return math::InvFourPi<float>;
        }
    else
        return math::InvFourPi<float>;
}

} // namespace misaki::warp