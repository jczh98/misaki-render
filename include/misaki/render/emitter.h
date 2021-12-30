#pragma once

#include <misaki/core/fwd.h>
#include "misaki/core/object.h"
#include "records.h"

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

class MSK_EXPORT Emitter : public Object{
public:

    // Returns Sampled ray with structred RaySample
    virtual std::pair<Ray, Spectrum>
    sample_ray(const Eigen::Vector2f &pos_sample,
               const Eigen::Vector2f &dir_sample) const;

    virtual Spectrum eval(const SceneInteraction &si) const;

    // Returns direction, weight
    virtual std::pair<PositionSample, Spectrum>
    sample_position(const Eigen::Vector2f &sample) const;
    virtual std::pair<DirectionSample, Spectrum>
    sample_direction(const PositionSample &ps,
                     const Eigen::Vector2f &sample) const;
    virtual std::pair<DirectIllumSample, Spectrum>
    sample_direct(const SceneInteraction &ref,
                  const Eigen::Vector2f &sample) const;

    virtual float pdf_position(const PositionSample &ps) const;
    virtual float pdf_direction(const PositionSample &ps,
                                const DirectionSample &ds) const;
    virtual float pdf_direct(const DirectIllumSample &ds) const;

    virtual void set_shape(Shape *shape);
    virtual void set_scene(const Scene *scene);
    virtual void set_medium(Medium *medium);

    Shape *shape() { return m_shape; }
    const Shape *shape() const { return m_shape; }

    Medium *medium() { return m_medium; }
    const Medium *medium() const { return m_medium; }

    bool is_environment() const {
        return has_flag(m_flags, EmitterFlags::Infinite) &&
               !has_flag(m_flags, EmitterFlags::Delta);
    }
    uint32_t flags() const { return m_flags; }

    MSK_DECLARE_CLASS()
protected:
    Emitter(const Properties &props);
    virtual ~Emitter();

protected:
    uint32_t m_flags;
    ref<Medium> m_medium;
    Transform4f m_world_transform;
    Shape *m_shape = nullptr;
    std::string m_id;
};

} // namespace misaki