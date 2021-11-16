#pragma once

#include "endpoint.h"
#include "film.h"
#include "fwd.h"
#include "object.h"
#include "ray.h"
#include "sampler.h"

namespace misaki {

class MSK_EXPORT Sensor : public Endpoint {
public:
    virtual std::pair<RayDifferential, Spectrum>
    sample_ray_differential(const Eigen::Vector2f &sample2,
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