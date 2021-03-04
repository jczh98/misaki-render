#pragma once

#include <sstream>

#include "math_helper.h"
#include "string.h"
#include <Eigen/Dense>

namespace aspirin {

template <typename Float> struct Frame {
    using Vector3 = Eigen::Matrix<Float, 3, 1>;
    using Vector2 = Eigen::Matrix<Float, 2, 1>;

    Vector3 s, t, n;

    Frame(const Vector3 &v) : n(v) {
        std::tie(s, t) = coordinate_system<Float>(v);
    }

    Vector3 to_local(const Vector3 &v) const {
        return { v.dot(s), v.dot(t), v.dot(n) };
    }

    Vector3 to_world(const Vector3 &v) const {
        return s * v.x() + t * v.y() + n * v.z();
    }

    std::string to_string() const {
        std::ostringstream os;
        os << "Frame[" << std::endl
           << "  s = " << string::indent(s.to_string(), 6) << "," << std::endl
           << "  t = " << string::indent(t.to_string(), 6) << "," << std::endl
           << "  n = " << string::indent(n.to_string(), 6) << std::endl
           << "]";
        return os.str();
    }

    static Float cos_theta(const Vector3 &v) { return v.z(); }
    static Float cos_theta_2(const Vector3 &v) { return math::sqr(v.z()); }
    static Float sin_theta(const Vector3 &v) {
        return math::safe_sqrt(sin_theta_2(v));
    }
    static Float sin_theta_2(const Vector3 &v) {
        return math::sqr(v.x()) + math::sqr(v.y());
    }
    static Float tan_theta(const Vector3 &v) {
        return math::safe_sqrt(1.f - math::sqr(v.z())) / v.z();
    }
    static Float tan_theta_2(const Vector3 &v) {
        return std::max(0.f, 1.f - math::sqr(v.z())) / math::sqr(v.z());
    }
    static Float sin_phi(const Vector3 &v) {
        Float sin_theta_2   = Frame::sin_theta_2(v),
              inv_sin_theta = math::safe_rsqrt(Frame::sin_theta_2(v));
        return std::abs(sin_theta_2) <= 4.f * math::Epsilon<Float>
                   ? 0.f
                   : std::clamp(v.y() * inv_sin_theta, -1.f, 1.f);
    }
    static Float cos_phi(const Vector3 &v) {
        Float sin_theta_2   = Frame::sin_theta_2(v),
              inv_sin_theta = math::safe_rsqrt(Frame::sin_theta_2(v));
        return std::abs(sin_theta_2) <= 4.f * math::Epsilon<Float>
                   ? 1.f
                   : std::clamp(v.x() * inv_sin_theta, -1.f, 1.f);
    }
    static std::pair<Float, Float> sincos_phi(const Vector3 &v) {
        Float sin_theta_2   = Frame::sin_theta_2(v),
              inv_sin_theta = math::safe_rsqrt(Frame::sin_theta_2(v));

        Vector2 result = { v.x() * inv_sin_theta, v.y() * inv_sin_theta };

        result = std::abs(sin_theta_2) <= 4.f * math::Epsilon<Float>
                     ? Vector2(1.f, 0.f)
                     : math::clamp(result, -1.f, 1.f);

        return { result.y(), result.x() };
    }
};

template <typename Float>
std::ostream &operator<<(std::ostream &os, const Frame<Float> &f) {
    os << "Frame[" << std::endl
       << "  s = " << string::indent(f.s.to_string(), 6) << "," << std::endl
       << "  t = " << string::indent(f.t.to_string(), 6) << "," << std::endl
       << "  n = " << string::indent(f.n.to_string(), 6) << std::endl
       << "]";
    return os;
}

} // namespace aspirin