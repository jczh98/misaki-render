#pragma once

#include "endpoint.h"
#include "fwd.h"

namespace misaki {

enum class EmitterFlags : uint32_t {
    None           = 0x00000,
    DeltaPosition  = 0x00001,
    DeltaDirection = 0x00002,
    Infinite       = 0x00004,
    Surface        = 0x00008,
    Delta          = DeltaPosition | DeltaDirection,
};

constexpr uint32_t operator|(EmitterFlags f1, EmitterFlags f2) {
    return (uint32_t) f1 | (uint32_t) f2;
}
constexpr uint32_t operator|(uint32_t f1, EmitterFlags f2) {
    return f1 | (uint32_t) f2;
}
constexpr uint32_t operator&(EmitterFlags f1, EmitterFlags f2) {
    return (uint32_t) f1 & (uint32_t) f2;
}
constexpr uint32_t operator&(uint32_t f1, EmitterFlags f2) {
    return f1 & (uint32_t) f2;
}
constexpr uint32_t operator~(EmitterFlags f1) { return ~(uint32_t) f1; }
constexpr uint32_t operator+(EmitterFlags e) { return (uint32_t) e; }
template <typename UInt32>
constexpr auto has_flag(UInt32 flags, EmitterFlags f) {
    return (flags & (uint32_t) f) != 0u;
}

class APR_EXPORT Emitter : public Endpoint {
public:
    bool is_environment() const {
        return has_flag(m_flags, EmitterFlags::Infinite) &&
               !has_flag(m_flags, EmitterFlags::Delta);
    }
    uint32_t flags() const { return m_flags; }

    APR_DECLARE_CLASS()
protected:
    Emitter(const Properties &props);
    virtual ~Emitter();

protected:
    uint32_t m_flags;
};

} // namespace misaki