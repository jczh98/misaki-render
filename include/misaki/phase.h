#pragma once

#include "bsdf.h"
#include "interaction.h"
#include "object.h"

namespace misaki {

enum class PhaseFunctionFlags : uint32_t {
    None        = 0x00,
    Isotropic   = 0x01,
    Anisotropic = 0x02,
    Microflake  = 0x04
};

constexpr uint32_t operator|(PhaseFunctionFlags f1, PhaseFunctionFlags f2) {
    return (uint32_t) f1 | (uint32_t) f2;
}
constexpr uint32_t operator|(uint32_t f1, PhaseFunctionFlags f2) {
    return f1 | (uint32_t) f2;
}
constexpr uint32_t operator&(PhaseFunctionFlags f1, PhaseFunctionFlags f2) {
    return (uint32_t) f1 & (uint32_t) f2;
}
constexpr uint32_t operator&(uint32_t f1, PhaseFunctionFlags f2) {
    return f1 & (uint32_t) f2;
}
constexpr uint32_t operator~(PhaseFunctionFlags f1) { return ~(uint32_t) f1; }
constexpr uint32_t operator+(PhaseFunctionFlags e) { return (uint32_t) e; }
template <typename UInt32>
constexpr auto has_flag(UInt32 flags, PhaseFunctionFlags f) {
    return (flags & (uint32_t) f) != 0;
}

struct APR_EXPORT PhaseFunctionContext {
    TransportMode mode;

    Sampler *sampler;

    PhaseFunctionContext(Sampler *sampler,
                         TransportMode mode = TransportMode::Radiance)
        : mode(mode), sampler(sampler) {}

    void reverse() { mode = (TransportMode)(1 - (int) mode); }
};

class APR_EXPORT PhaseFunction : public Object {
public:
    /**
     * @brief Sample a phase function
     * @param sample
     * @return sampled direction, pdf, and phase value
     */
    virtual std::tuple<Vector3, Float, Float>
    sample(const PhaseFunctionContext &ctx, const MediumInteraction &mi,
           const Vector2 &sample) const = 0;

    virtual Float eval(const PhaseFunctionContext &ctx,
                       const MediumInteraction &mi,
                       const Vector3 &wo) const = 0;

    uint32_t flags() const { return m_flags; }

    std::string id() const override { return m_id; }

    std::string to_string() const override = 0;

    APR_DECLARE_CLASS()
protected:
    PhaseFunction(const Properties &props);
    virtual ~PhaseFunction();

protected:
    uint32_t m_flags;

    std::string m_id;
};

} // namespace misaki