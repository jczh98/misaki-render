#pragma once

#include <optional>

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include "records.h"

namespace misaki {

class MSK_EXPORT Endpoint : public Object {
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
    sample_direct(const SceneInteraction &ref, const Eigen::Vector2f &sample) const;

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

    MSK_DECLARE_CLASS()
protected:
    Endpoint(const Properties &props);

    virtual ~Endpoint();

protected:
    ref<Medium> m_medium;
    Transform4f m_world_transform;
    Shape *m_shape = nullptr;
    std::string m_id;
};

} // namespace misaki