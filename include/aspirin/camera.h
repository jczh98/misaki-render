#pragma once

#include "component.h"
#include "endpoint.h"
#include "film.h"
#include "fwd.h"
#include "ray.h"
#include "sampler.h"

namespace aspirin {

template <typename Spectrum>
class APR_EXPORT Camera : public Endpoint<Spectrum> {
public:
    using Ray     = Ray<Spectrum>;
    using Film    = Film<Spectrum>;
    using Sampler = Sampler<Spectrum>;

    Camera(const Properties &);

    std::pair<Ray, Vector3>
    sample_ray(const Vector2 &pos_sample) const override;

    Film *film() { return m_film.get(); }
    const Film *film() const { return m_film.get(); }
    Sampler *sampler() { return m_sampler.get(); }
    const Sampler *sampler() const { return m_sampler.get(); }

protected:
    std::shared_ptr<Film> m_film;
    std::shared_ptr<Sampler> m_sampler;
    Vector2 m_resolution;
    Float m_aspect;
};

template <typename Spectrum>
class APR_EXPORT ProjectiveCamera : public Camera<Spectrum> {
public:
    ProjectiveCamera(const Properties &props);
    Float near_clip() const { return m_near_clip; }
    Float far_clip() const { return m_far_clip; }
    Float focus_distance() const { return m_focus_distance; }

protected:
    Float m_near_clip, m_far_clip, m_focus_distance;
};

} // namespace aspirin