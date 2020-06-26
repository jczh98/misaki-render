#pragma once

#include "component.h"
#include "endpoint.h"
#include "film.h"
#include "fwd.h"
#include "ray.h"
#include "sampler.h"

namespace misaki::render {

class MSK_EXPORT Camera : public Endpoint {
 public:
  Camera(const Properties&);

  virtual std::pair<Ray, Vector3> sample_ray(const Vector2& pos_sample) const;

  Film* film() { return m_film.get(); }
  const Film* film() const { return m_film.get(); }
  Sampler* sampler() { return m_sampler.get(); }
  const Sampler* sampler() const { return m_sampler.get(); }

  MSK_DECL_COMP(Endpoint)
 protected:
  std::shared_ptr<Film> m_film;
  std::shared_ptr<Sampler> m_sampler;
  Vector2 m_resolution;
  Float m_aspect;
};

class MSK_EXPORT ProjectiveCamera : public Camera {
 public:
  ProjectiveCamera(const Properties& props);
  Float near_clip() const { return m_near_clip; }
  Float far_clip() const { return m_far_clip; }
  Float focus_distance() const { return m_focus_distance; }

  MSK_DECL_COMP(Camera)
 protected:
  Float m_near_clip, m_far_clip, m_focus_distance;
};

}  // namespace misaki::render