#pragma once

#include <Eigen/Core>
#include "platform.h"

namespace misaki {

namespace math {

template <typename T> constexpr auto Pi        = T(3.14159265358979323846);
template <typename T> constexpr auto InvPi     = T(0.31830988618379067154);
template <typename T> constexpr auto InvTwoPi  = T(0.15915494309189533577);
template <typename T> constexpr auto InvFourPi = T(0.07957747154594766788);
template <typename T>
constexpr auto Infinity = std::numeric_limits<T>::infinity();
template <typename T>
constexpr auto Epsilon = std::numeric_limits<T>::epsilon() / 2;

template <typename T> constexpr auto RayEpsilon    = Epsilon<T> * 1500;
template <typename T> constexpr auto ShadowEpsilon = RayEpsilon<T> * 10;

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

MSK_INLINE Eigen::VectorXf clamp(const Eigen::VectorXf &v, float vmin, float vmax) {
    return v.cwiseMin(vmax).cwiseMax(vmin);
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

inline std::pair<float, float> sincos(const float& v) {
    return { std::sinf(v), std::cosf(v) };
}

// Search the interval of [begin, end)
template <typename Predicate>
uint32_t binary_search(uint32_t begin, uint32_t end, const Predicate &pred) {
    while (begin < end) {
        uint32_t middle = begin + ((end - begin) >> 1);
        if (pred(middle)) {
            begin = middle + 1;
        } else
            end = middle;
    }
    return begin;
}

#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
#define PCG32_MULT 0x5851f42d4c957f2dULL

struct PCG32 {
    PCG32() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}
    PCG32(uint64_t initstate, uint64_t initseq = 1u) {
        seed(initstate, initseq);
    }

    void seed(uint64_t initstate, uint64_t initseq = 1) {
        state = 0U;
        inc   = (initseq << 1u) | 1u;
        next_uint32();
        state += initstate;
        next_uint32();
    }
    uint32_t next_uint32() {
        uint64_t oldstate = state;
        state             = oldstate * PCG32_MULT + inc;
        uint32_t xorshifted =
            (uint32_t) (((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot = (uint32_t) (oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
    }

    float next_float32() {
        /* Trick from MTGP: generate an uniformly distributed
               single precision number in [1,2) and subtract 1. */
        union {
            uint32_t u;
            float f;
        } x;
        x.u = (next_uint32() >> 9) | 0x3f800000u;
        return x.f - 1.0f;
    }

    double next_float64() {
        /* Trick from MTGP: generate an uniformly distributed
               double precision number in [1,2) and subtract 1. */
        union {
            uint64_t u;
            double d;
        } x;
        x.u = ((uint64_t) next_uint32() << 20) | 0x3ff0000000000000ULL;
        return x.d - 1.0;
    }

    bool operator==(const PCG32 &other) const {
        return state == other.state && inc == other.inc;
    }
    bool operator!=(const PCG32 &other) const {
        return state != other.state || inc != other.inc;
    }

    uint64_t state; // RNG state.  All values are possible.
    uint64_t inc;   // Controls which RNG sequence (stream) is selected. Must
                    // *always* be odd.
};

} // namespace math

namespace math {

template <typename T, int D>
Eigen::Matrix<int, D, 1> ceil2int(const Eigen::Matrix<T, D, 1> &vec) {
    return vec.array().ceil().matrix().template cast<int>();
}

template <typename T, int D>
Eigen::Matrix<int, D, 1> floor2int(const Eigen::Matrix<T, D, 1> &vec) {
    return vec.array().floor().matrix().template cast<int>();
}

MSK_INLINE Eigen::RowVectorXi ceil2int(const Eigen::RowVectorXf &vec) {
    return vec.array().ceil().matrix().template cast<int>();
}

MSK_INLINE Eigen::RowVectorXi floor2int(const Eigen::RowVectorXf &vec) {
    return vec.array().floor().matrix().template cast<int>();
}

template <typename Array, size_t... Index, typename Value = typename Array::Scalar>
MSK_INLINE Array sample_shifted(const Value &sample,
                                  std::index_sequence<Index...>) {
    const Array shift(Index / Array::Scalar(Array::SizeAtCompileTime)...);

    Array value = Array::Constant(sample) + shift;
    Array one   = Array::Constant(1);
    Array result = (value <= one).select(value, value - one);

    return result;
}

template <typename Array>
MSK_INLINE Array sample_shifted(const typename Array::Scalar &sample) {
    return sample_shifted<Array>(sample, std::make_index_sequence<Array::SizeAtCompileTime>());
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

MSK_INLINE std::pair<Eigen::Vector3f, Eigen::Vector3f>
coordinate_system(const Eigen::Vector3f &n) {
    float sign    = std::copysign(1.f, n.z());
    const float a = -1.f / (sign + n.z());
    const float b = n.x() * n.y() * a;
    return { { 1.f + sign * n.x() * n.x() * a, sign * b, -sign * n.x() },
             { b, sign + n.y() * n.y() * a, -n.y() } };
}

template <typename T, int D>
std::ostream &operator<<(std::ostream &out, const Eigen::Matrix<T, D, 1> &vec) {
    std::string result;
    for (size_t i = 0; i < D; ++i) {
        result += std::to_string(vec.coeff(i));
        result += i + 1 < D ? ", " : "";
    }
    out << "[" + result + "]";
    return out;
}

template <typename T, int D>
std::ostream &operator<<(std::ostream &out, const Eigen::Array<T, D, 1> &vec) {
    std::string result;
    for (size_t i = 0; i < D; ++i) {
        result += std::to_string(vec.coeff(i));
        result += i + 1 < D ? ", " : "";
    }
    out << "[" + result + "]";
    return out;
}

template <typename T, int D>
std::ostream &operator<<(std::ostream &os, const Eigen::Matrix<T, D, D> &m) {
    os << m.format(Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
    return os;
}

inline uint32_t float_to_bits(float f) {
    uint32_t ui;
    memcpy(&ui, &f, sizeof(float));
    return ui;
}

inline float bits_to_float(uint32_t ui) {
    float f;
    memcpy(&f, &ui, sizeof(uint32_t));
    return f;
}

inline uint64_t double_to_bits(double f) {
    uint64_t ui;
    memcpy(&ui, &f, sizeof(double));
    return ui;
}

inline double bits_to_double(uint64_t ui) {
    double f;
    memcpy(&f, &ui, sizeof(uint64_t));
    return f;
}

} // namespace misaki