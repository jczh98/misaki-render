#pragma once

#include "misaki/core/object.h"
#include "records.h"
#include "film.h"
#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include "misaki/core/ray.h"
#include "sampler.h"

namespace misaki {

class MSK_EXPORT Sensor : public Object {
public:
    // Returns Sampled ray with structred RaySample
    virtual std::pair<Ray, Spectrum>
    sample_ray(const float wavelength_sample,
               const Eigen::Vector2f &pos_sample,
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

    virtual std::pair<RayDifferential, Spectrum>
    sample_ray_differential(const float wavelength_sample,
                            const Eigen::Vector2f &sample2,
                            const Eigen::Vector2f &sample3) const;

    Film *film() { return m_film; }
    const Film *film() const { return m_film; }
    Sampler *sampler() { return m_sampler; }
    const Sampler *sampler() const { return m_sampler; }

    MSK_DECLARE_CLASS()
protected:
    Sensor(const Properties &);

    virtual ~Sensor();

protected:
    ref<Medium> m_medium;
    Transform4f m_world_transform;
    Shape *m_shape = nullptr;
    std::string m_id;

    ref<Film> m_film;
    ref<Sampler> m_sampler;
    Eigen::Vector2f m_resolution;
    float m_aspect;
};

class MSK_EXPORT ProjectiveCamera : public Sensor {
public:
    float near_clip() const { return m_near_clip; }
    float far_clip() const { return m_far_clip; }
    float focus_distance() const { return m_focus_distance; }

    MSK_DECLARE_CLASS()
protected:
    ProjectiveCamera(const Properties &props);

    virtual ~ProjectiveCamera();

protected:
    float m_near_clip, m_far_clip, m_focus_distance;
};

} // namespace misaki
