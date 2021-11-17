#pragma once

#include "fwd.h"
#include "math_helper.h"

namespace misaki {

template <typename Float> class AtomicFloat {
public:
    explicit AtomicFloat(Float v = 0) {
        if constexpr (std::is_same_v<Float, float>) {
            m_32bits = float_to_bits(v);
        } else {
            m_64bits = double_to_bits(v);
        }
    }

    operator Float() const {
        if constexpr (std::is_same_v<Float, float>) {
            return bits_to_float(m_32bits);
        } else {
            return bits_to_double(m_64bits);
        }
    }

    Float operator=(Float v) {
        if constexpr (std::is_same_v<Float, float>) {
            m_32bits = float_to_bits(v);
        } else {
            m_64bits = double_to_bits(v);
        }
        return v;
    }

    void add(Float v) {
        if constexpr (std::is_same_v<Float, float>) {
            uint32_t old_bits = m_32bits, new_bits;
            do {
                new_bits = float_to_bits(bits_to_float(old_bits) + v);
            } while (!m_32bits.compare_exchange_weak(old_bits, new_bits));
        } else {
            uint64_t old_bits = m_64bits, new_bits;
            do {
                new_bits = double_to_bits(bits_to_double(old_bits) + v);
            } while (!m_64bits.compare_exchange_weak(old_bits, new_bits));
        }
    }

private:
    std::atomic<uint64_t> m_64bits;
    std::atomic<uint32_t> m_32bits;
};

} // namespace misaki