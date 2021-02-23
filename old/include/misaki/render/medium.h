#pragma once

#include "component.h"
#include "phase.h"

namespace misaki::render {

class MSK_EXPORT Medium : public Component {
 public:
  Medium(const Proerties& props);

  SceneInteraction sample_interaction(const Ray& ray, Float sample) const;

  MSK_DECL_COMP(Component)
 protected:
  std::shared_ptr<PhaseFunction> m_phase_function;
  std::string m_id;
};

}  // namespace misaki::render