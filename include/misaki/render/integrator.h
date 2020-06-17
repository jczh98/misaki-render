#pragma once

#include "component.h"

namespace misaki::render {

class Integrator : public Component {
 public:
  Integrator(const Properties &props);
  virtual bool render(Scene *scene) = 0;

  MSK_DECL_COMP(Component)
};

}  // namespace misaki::render