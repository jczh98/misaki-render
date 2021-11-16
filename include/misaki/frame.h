#pragma once

#include <sstream>

#include "mathutils.h"
#include "string.h"
#include <Eigen/Core>

namespace misaki {

struct Frame {
    Eigen::Vector3f s, t, n;

    Frame(const Eigen::Vector3f &v) : n(v) {
        std::tie(s, t) = coordinate_system<float>(v);
    }

    Eigen::Vector3f to_local(const Eigen::Vector3f &v) const {
        return { v.dot(s), v.dot(t), v.dot(n) };
    }

    Eigen::Vector3f to_world(const Eigen::Vector3f &v) const {
        return s * v.x() + t * v.y() + n * v.z();
    }

    std::string to_string() const {
        std::ostringstream os;
        os << "Frame[" << std::endl
           << "  s = " << s << "," << std::endl
           << "  t = " << t << "," << std::endl
           << "  n = " << n << std::endl
           << "]";
        return os.str();
    }

    static float cos_theta(const Eigen::Vector3f &v) { return v.z(); }
    static float cos_theta_2(const Eigen::Vector3f &v) {
        return math::sqr(v.z());
    }
    static float sin_theta(const Eigen::Vector3f &v) {
        return math::safe_sqrt(sin_theta_2(v));
    }
    static float sin_theta_2(const Eigen::Vector3f &v) {
        return math::sqr(v.x()) + math::sqr(v.y());
    }
    static float tan_theta(const Eigen::Vector3f &v) {
        return math::safe_sqrt(1.f - math::sqr(v.z())) / v.z();
    }
    static float tan_theta_2(const Eigen::Vector3f &v) {
        return std::max(0.f, 1.f - math::sqr(v.z())) / math::sqr(v.z());
    }
    static float sin_phi(const Eigen::Vector3f &v) {
        float sin_theta_2   = Frame::sin_theta_2(v),
              inv_sin_theta = math::safe_rsqrt(Frame::sin_theta_2(v));
        return std::abs(sin_theta_2) <= 4.f * math::Epsilon<float>
                   ? 0.f
                   : std::clamp(v.y() * inv_sin_theta, -1.f, 1.f);
    }
    static float cos_phi(const Eigen::Vector3f &v) {
        float sin_theta_2   = Frame::sin_theta_2(v),
              inv_sin_theta = math::safe_rsqrt(Frame::sin_theta_2(v));
        return std::abs(sin_theta_2) <= 4.f * math::Epsilon<float>
                   ? 1.f
                   : std::clamp(v.x() * inv_sin_theta, -1.f, 1.f);
    }
    static std::pair<float, float> sincos_phi(const Eigen::Vector3f &v) {
        float sin_theta_2   = Frame::sin_theta_2(v),
              inv_sin_theta = math::safe_rsqrt(Frame::sin_theta_2(v));

        Eigen::Vector2f result = { v.x() * inv_sin_theta,
                                      v.y() * inv_sin_theta };

        result = std::abs(sin_theta_2) <= 4.f * math::Epsilon<float>
                     ? Eigen::Vector2f(1.f, 0.f)
                     :  math::clamp(result, -1.f, 1.f);

        return { result.y(), result.x() };
    }
};

MSK_INLINE std::ostream &operator<<(std::ostream &os, const Frame &f) {
    os << "Frame[" << std::endl
       << "  s = " << f.s << "," << std::endl
       << "  t = " << f.t << "," << std::endl
       << "  n = " << f.n << std::endl
       << "]";
    return os;
}

} // namespace misaki