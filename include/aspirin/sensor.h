#pragma once

#include "endpoint.h"
#include "film.h"
#include "fwd.h"
#include "object.h"
#include "ray.h"
#include "sampler.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Sensor : public Endpoint<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Endpoint<Float, Spectrum>;
    using Base::sample_ray;
    using typename Base::Ray;
    using RayDifferential = RayDifferential<Float, Spectrum>;
    using Film            = Film<Float, Spectrum>;
    using Sampler         = Sampler<Float, Spectrum>;

    virtual std::pair<RayDifferential, Spectrum>
    sample_ray_differential(const Vector2 &sample2) const;

    Film *film() { return m_film; }
    const Film *film() const { return m_film; }
    Sampler *sampler() { return m_sampler; }
    const Sampler *sampler() const { return m_sampler; }

    APR_DECLARE_CLASS()
protected:
    Sensor(const Properties &);

    virtual ~Sensor();

protected:
    ref<Film> m_film;
    ref<Sampler> m_sampler;
    Vector2 m_resolution;
    Float m_aspect;
};

template <typename Float, typename Spectrum>
class APR_EXPORT ProjectiveCamera : public Sensor<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)

    Float near_clip() const { return m_near_clip; }
    Float far_clip() const { return m_far_clip; }
    Float focus_distance() const { return m_focus_distance; }

    APR_DECLARE_CLASS()
protected:
    ProjectiveCamera(const Properties &props);

    virtual ~ProjectiveCamera();

protected:
    Float m_near_clip, m_far_clip, m_focus_distance;
};

APR_EXTERN_CLASS(Sensor)
APR_EXTERN_CLASS(ProjectiveCamera)

} // namespace aspirin