#pragma once

#include "component.h"

namespace misaki::render {

class MSK_EXPORT Integrator : public Component {
 public:
  Integrator(const Properties &props);
  virtual bool render(const std::shared_ptr<Scene> &scene);

  MSK_DECL_COMP(Component)
 protected:
  uint32_t m_block_size;
};

}  // namespace misaki::render