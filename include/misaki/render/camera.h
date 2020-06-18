#pragma once

#include "component.h"
#include "film.h"
#include "fwd.h"
#include "sampler.h"

namespace misaki::render {

class MSK_EXPORT Camera : public Component {
 public:
  Camera(const Properties&);

  Film* film() { return m_film.get(); }
  const Film* film() const { return m_film.get(); }
  Sampler* sampler() { return m_sampler.get(); }
  const Sampler* sampler() const { return m_sampler.get(); }

  MSK_DECL_COMP(Component)
 protected:
  std::shared_ptr<Film> m_film;
  std::shared_ptr<Sampler> m_sampler;
  Vector2 m_resolution;
  Float m_aspect;
};

}  // namespace misaki::render