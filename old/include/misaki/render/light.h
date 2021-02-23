#pragma once

#include "endpoint.h"
#include "fwd.h"

namespace misaki::render {

enum class LightFlags : uint32_t {
  None = 0x00000,
  DeltaPosition = 0x00001,
  DeltaDirection = 0x00002,
  Infinite = 0x00004,
  Surface = 0x00008,
  Delta = DeltaPosition | DeltaDirection,
};

constexpr uint32_t operator|(LightFlags f1, LightFlags f2) { return (uint32_t)f1 | (uint32_t)f2; }
constexpr uint32_t operator|(uint32_t f1, LightFlags f2) { return f1 | (uint32_t)f2; }
constexpr uint32_t operator&(LightFlags f1, LightFlags f2) { return (uint32_t)f1 & (uint32_t)f2; }
constexpr uint32_t operator&(uint32_t f1, LightFlags f2) { return f1 & (uint32_t)f2; }
constexpr uint32_t operator~(LightFlags f1) { return ~(uint32_t)f1; }
constexpr uint32_t operator+(LightFlags e) { return (uint32_t)e; }
template <typename UInt32>
constexpr auto has_flag(UInt32 flags, LightFlags f) { return (flags & (uint32_t)f) != 0u; }

class MSK_EXPORT Light : public Endpoint {
 public:
  Light(const Properties &props);
  bool is_environment() const {
    return has_flag(m_flags, LightFlags::Infinite) && !has_flag(m_flags, LightFlags::Delta);
  }
  uint32_t flags() const { return m_flags; }

 protected:
  uint32_t m_flags;
};

}  // namespace misaki::render