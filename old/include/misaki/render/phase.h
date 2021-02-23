#pragma once

#include "bsdf.h"
#include "component.h"
#include "fwd.h"

namespace misaki::render {

enum class PhaseFunctionFlags : uint32_t {
  None = 0x00,
  Isotropic = 0x01,
  Anisotropic = 0x02
};

constexpr uint32_t operator|(PhaseFunctionFlags f1, PhaseFunctionFlags f2) {
  return (uint32_t)f1 | (uint32_t)f2;
}
constexpr uint32_t operator|(uint32_t f1, PhaseFunctionFlags f2) { return f1 | (uint32_t)f2; }
constexpr uint32_t operator&(PhaseFunctionFlags f1, PhaseFunctionFlags f2) {
  return (uint32_t)f1 & (uint32_t)f2;
}
constexpr uint32_t operator&(uint32_t f1, PhaseFunctionFlags f2) { return f1 & (uint32_t)f2; }
constexpr uint32_t operator~(PhaseFunctionFlags f1) { return ~(uint32_t)f1; }
constexpr uint32_t operator+(PhaseFunctionFlags e) { return (uint32_t)e; }
constexpr auto has_flag(uint32_t flags, PhaseFunctionFlags f) {
  return (flags & (uint32_t)f) != 0;
}

struct MSK_EXPORT PhaseFunctionContext {
  TransportMode mode;
  PhaseFunctionContext(TransportMode mode = TransportMode::Radiance)
      : mode(mode) {}
};

struct MSK_EXPORT PhaseFunction : public Component {
 public:
  PhaseFunction(const Properties &props);
  virtual std::pair<Vector3f, Float> sample(const PhaseFunctionContext &ctx,
                                            const SceneInteraction &mi,
                                            const Vector2 &sample) const;
  virtual Float eval(const PhaseFunctionContext &ctx, const SceneInteraction &mi,
                     const Vector3 &wo) const;

  uint32_t flags() const { return m_flags; }
  std::string id() const { return m_id; }

  MSK_DECL_COMP(Component)
 protected:
  uint32_t m_flags;
  std::string m_id;
};
}  // namespace misaki::render