#pragma once

#include <Eigen/Dense>

namespace aspirin {

namespace math {

template <typename T> constexpr auto Pi        = T(3.14159265358979323846);
template <typename T> constexpr auto InvPi     = T(0.31830988618379067154);
template <typename T> constexpr auto InvTwoPi  = T(0.15915494309189533577);
template <typename T> constexpr auto InvFourPi = T(0.07957747154594766788);
template <typename T>
constexpr auto Infinity = std::numeric_limits<T>::infinity();
template <typename T>
constexpr auto Epsilon = std::numeric_limits<T>::epsilon() / 2;

template <typename T>
constexpr auto RayEpsilon = Epsilon<T> * 1500;
template <typename T>
constexpr auto ShadowEpsilon = RayEpsilon<T> * 10;

template <typename T> inline auto deg_to_rag(const T &v) {
    return v * T(Pi<T> / 180);
}

template <typename T> inline auto rag_to_deg(const T &v) {
    return v * T(180 / Pi<T>);
}

template <int N, typename T> T power(const T &x) {
    if constexpr (N == 0) {
        return T(1);
    } else if constexpr (N % 2 == 0) {
        auto tmp = power<N / 2>(x);
        return tmp * tmp;
    } else {
        auto tmp = power<N / 2>(x);
        return tmp * tmp * x;
    }
}

template <typename T> T clamp(T value, T vmin, T vmax) {
    return value < vmin ? vmin : (value > vmax ? vmax : value);
}

template <typename T> inline T sqr(const T &a) { return a * a; }

template <typename T> inline auto safe_sqrt(const T &a) {
    return std::sqrt(std::max(a, T(0)));
}

template <typename T> inline auto safe_rsqrt(const T &a) {
    return T(1) / std::sqrt(std::max(a, T(0)));
}

template <typename T> inline auto safe_asin(const T &a) {
    return std::asin(std::min(T(1), std::max(T(-1), a)));
}

template <typename T> inline auto safe_acos(const T &a) {
    return std::acos(std::min(T(1), std::max(T(-1), a)));
}

// Search the interval of [begin, end)
template <typename Predicate>
uint32_t binary_search(uint32_t begin, uint32_t end, const Predicate &pred) {
    while (begin < end) {
        uint32_t middle = begin + (end - begin) >> 1;
        if (pred(middle)) {
            begin = middle + 1;
        } else
            end = middle;
    }
    return begin;
}

} // namespace math

template <typename Float>
std::pair<Eigen::Matrix<Float, 3, 1>, Eigen::Matrix<Float, 3, 1>>
coordinate_system(const Eigen::Matrix<Float, 3, 1> &n) {
    Float sign    = std::copysign(1.f, n.z());
    const Float a = -1.f / (sign + n.z());
    const Float b = n.x() * n.y() * a;
    return { { 1.f + sign * n.x() * n.x() * a, sign * b, -sign * n.x() },
             { b, sign + n.y() * n.y() * a, -n.y() } };
}

} // namespace aspirin