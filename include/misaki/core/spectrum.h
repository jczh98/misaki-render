#pragma once

#include "platform.h"
#include <Eigen/Core>
#include "mathutils.h"

namespace misaki {

#if !defined(MSK_WAVELENGTH_MIN)
#define MSK_WAVELENGTH_MIN 360.f
#endif

#if !defined(MSK_WAVELENGTH_MAX)
#define MSK_WAVELENGTH_MAX 830.f
#endif

template <typename Value_, size_t Size_ = 3> struct Color
    : public Eigen::Array<Value_, Size_, 1> {
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

template <typename Value_, size_t Size_ = 4> struct SpectrumArray
    : public Eigen::Array<Value_, Size_, 1> {
    using Base = Eigen::Array<Value_, Size_, 1>;
    using Base::Base;
    using Base::operator=;

    bool operator!=(const SpectrumArray<Value_, Size_> &rhs) const {
        return Base::operator!=(rhs).all();
    }
};

template <typename Value_, size_t Size_ = 3> bool is_black(
    const Color<Value_, Size_> &col) {
    bool result = true;
    for (size_t i = 0; i < Size_; i++)
        if (col.coeff(i) != Value_(0.f)) {
            result = false;
        }
    return result;
}

template <typename Value_, size_t Size_ = 4> bool is_black(
    const SpectrumArray<Value_, Size_> &col) {
    bool result = true;
    for (size_t i = 0; i < Size_; i++)
        if (col.coeff(i) != Value_(0.f)) {
            result = false;
        }
    return result;
}

#define MSK_CIE_MIN 360.f
#define MSK_CIE_MAX 830.f
#define MSK_CIE_SAMPLES 95

#define MSK_CIE_Y_NORMALIZATION float(1.0 / 106.7502593994140625)

/// Table with fits for \ref cie1931_xyz and \ref cie1931_y
extern MSK_EXPORT const float *cie1931_x_data;
extern MSK_EXPORT const float *cie1931_y_data;
extern MSK_EXPORT const float *cie1931_z_data;

template <size_t Size> Color<SpectrumArray<float, Size>, 3>
cie1931_xyz(SpectrumArray<float, Size> wavelengths) {
    using Array = SpectrumArray<float, Size>;
    Array t =
        (wavelengths - Array::Constant(MSK_CIE_MIN)) *
        (Array::Constant((MSK_CIE_SAMPLES - 1) / (MSK_CIE_MAX - MSK_CIE_MIN)));

    Array w1, w0, v0_x, v1_x, v0_y, v1_y, v0_z, v1_z;
    for (int s = 0; s < Array::MaxRowsAtCompileTime; s++) {
        uint32_t i0 = std::clamp(uint32_t(t.coeff(s)), 0u,
                                 uint32_t(MSK_CIE_SAMPLES - 2)),
                 i1      = i0 + 1;
        v0_x.coeffRef(s) = cie1931_x_data[i0];
        v1_x.coeffRef(s) = cie1931_x_data[i1];
        v0_y.coeffRef(s) = cie1931_y_data[i0];
        v1_y.coeffRef(s) = cie1931_y_data[i1];
        v0_z.coeffRef(s) = cie1931_z_data[i0];
        v1_z.coeffRef(s) = cie1931_z_data[i1];

        w1.coeffRef(s) = t.coeff(s) - float(i0);
        w0.coeffRef(s) = 1.f - w1.coeff(s);
    }

    return Color<Array, 3>(w0 * v0_x + w1 * v1_x, w0 * v0_y + w1 * v1_y,
                           w0 * v0_z + w1 * v1_z);
}

template <size_t Size> Color<float, 3> spectrum_to_xyz(
    const SpectrumArray<float, Size> &value,
    const SpectrumArray<float, Size> &wavelengths) {
    Color<SpectrumArray<float, Size>, 3> XYZ = cie1931_xyz(wavelengths);
    return { (XYZ.x() * value).mean(), (XYZ.y() * value).mean(),
             (XYZ.z() * value).mean() };
}

template <typename T, int D> std::ostream &operator<<(
    std::ostream &out, const Color<T, D> &c) {
    std::string result;
    for (size_t i = 0; i < D; ++i) {
        result += std::to_string(c.coeff(i));
        result += i + 1 < D ? ", " : "";
    }
    out << "[" + result + "]";
    return out;
}

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

template <typename Value, size_t Size = 4> std::pair<
    SpectrumArray<Value, Size>, SpectrumArray<Value, Size>>
sample_uniform_spectrum(const SpectrumArray<Value, Size> &sample) {
    return { sample * (MSK_CIE_MAX - MSK_CIE_MIN) + MSK_CIE_MIN,
             SpectrumArray<Value, Size>::Constant(MSK_CIE_MAX - MSK_CIE_MIN) };
}

template <typename Value, size_t Size = 4> std::pair<
    SpectrumArray<Value, Size>, SpectrumArray<Value, Size>>
sample_rgb_spectrum(const SpectrumArray<Value, Size> &sample) {
    if constexpr (MSK_WAVELENGTH_MIN == 360.f && MSK_WAVELENGTH_MAX == 830.f) {
        SpectrumArray<Value, Size> wavelengths =
            538.f -
            (0.8569106254698279f - 1.8275019724092267f * sample).unaryExpr(
                [](Value x) {
                    return std::atanh(x);
                }) *
            138.88888888888889f;

        SpectrumArray<Value, Size> tmp = (0.0072f * (wavelengths - 538.f)).
            cosh();
        SpectrumArray<Value, Size> weight = 253.82f * tmp * tmp;

        return { wavelengths, weight };
    } else {
        // Fall back to uniform sampling for other wavelength ranges
        return sample_uniform_spectrum(sample);
    }
}

template <typename Value, size_t Size = 4> std::pair<
    SpectrumArray<Value, Size>, SpectrumArray<Value, Size>>
sample_wavelength(Value sample) {
    SpectrumArray<Value, Size> wav_sample =
        math::sample_shifted<SpectrumArray<Value, Size>>(sample);
    return sample_rgb_spectrum(wav_sample);
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
