#pragma once

#include <Eigen/Core>

namespace misaki {

template <typename Value_, size_t Size_ = 3>
struct Color : public Eigen::Array<Value_, Size_, 1> {
    using Base = Eigen::Array<Value_, Size_, 1>;
    using Base::Base;
    using Base::operator=;

    decltype(auto) r() const { return Base::x(); }
    decltype(auto) r() { return Base::x(); }

    decltype(auto) g() const { return Base::y(); }
    decltype(auto) g() { return Base::y(); }

    decltype(auto) b() const { return Base::z(); }
    decltype(auto) b() { return Base::z(); }

    decltype(auto) a() const { return Base::w(); }
    decltype(auto) a() { return Base::w(); }

    bool operator!=(const Color<Value_, Size_> &rhs) const {
        return Base::operator!=(rhs).all();
    }
};

template <typename Value_, size_t Size_ = 3>
bool is_black(const Color<Value_, Size_> &col) {
    bool result = true;
    for (size_t i = 0; i < Size_; i++)
        if (col.coeff(i) != Value_(0.f)) {
            result = false;
        }
    return result;
}

template <typename T, int D>
std::ostream &operator<<(std::ostream &out, const Color<T, D> &c) {
    std::string result;
    for (size_t i = 0; i < D; ++i) {
        result += std::to_string(c.coeff(i));
        result += i + 1 < D ? ", " : "";
    }
    out << "[" + result + "]";
    return out;
}

namespace detail {

template <typename Spectrum> struct spectrum_traits {};

template <typename Float> struct spectrum_traits<Color<Float, 1>> {
    using Scalar                           = Color<Float, 1>;
    using Wavelength                       = Color<Float, 1>;
    static constexpr bool is_monochromatic = true;
    static constexpr bool is_rgb           = false;
    static constexpr bool is_spectral      = false;
};

template <typename Float> struct spectrum_traits<Color<Float, 3>> {
    using Scalar                           = Color<Float, 3>;
    using Wavelength                       = Color<Float, 0>;
    static constexpr bool is_monochromatic = false;
    static constexpr bool is_rgb           = true;
    static constexpr bool is_spectral      = false;
};

} // namespace detail

template <typename T>
constexpr bool is_monochromatic_v =
    detail::spectrum_traits<T>::is_monochromatic;
template <typename T>
constexpr bool is_rgb_v = detail::spectrum_traits<T>::is_rgb;
template <typename T>
constexpr bool is_spectral_v = detail::spectrum_traits<T>::is_spectral;

// Copy from
// https://github.com/mitsuba-renderer/mitsuba2/blob/master/include/mitsuba/core/spectrum.h

inline Eigen::Vector3f srgb_to_xyz(const Eigen::Vector3f &rgb) {
    Eigen::Matrix3f M;
    M << 0.412453f, 0.357580f, 0.180423f, 0.212671f, 0.715160f, 0.072169f,
        0.019334f, 0.119193f, 0.950227f;
    return M * rgb;
}

inline Eigen::Vector3f xyz_to_srgb(const Eigen::Vector3f &rgb) {
    Eigen::Matrix3f M;
    M << 3.240479f, -1.537150f, -0.498535f, -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f;
    return M * rgb;
}

//// Convert ITU-R Rec. BT.709 linear RGB to XYZ tristimulus values
// template <typename Float>
// Color<Float, 3> srgb_to_xyz(const Color<Float, 3> &rgb) {
//    using Matrix3f = Eigen::Matrix<Float, 3, 3>;
//    Matrix3f M;
//    M << 0.412453f, 0.357580f, 0.180423f, 0.212671f, 0.715160f, 0.072169f,
//        0.019334f, 0.119193f, 0.950227f;
//    return M * rgb;
//}
//
//// Convert XYZ tristimulus values to ITU-R Rec. BT.709 linear RGB
// template <typename Float>
// Color<Float, 3> xyz_to_srgb(const Color<Float, 3> &rgb) {
//    using Matrix3f = Eigen::Matrix<Float, 3, 3>;
//    Matrix3f M;
//    M << 3.240479f, -1.537150f, -0.498535f, -0.969256f, 1.875991f, 0.041556f,
//        0.055648f, -0.204043f, 1.057311f;
//    return M * rgb;
//}
//
// template <typename Float> Float luminance(const Color<Float, 3> &c) {
//    return c[0] * 0.212671f + c[1] * 0.715160f + c[2] * 0.072169f;
//}

} // namespace misaki