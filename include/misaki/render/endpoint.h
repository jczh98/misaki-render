#pragma once

#include "component.h"

namespace misaki::render {

class MSK_EXPORT Endpoint : public Component {
 public:
  Endpoint(const Properties &props);

  MSK_DECL_COMP(Component)
 protected:
  Transform4 m_world_transform;
  std::string m_id;
};

}  // namespace misaki::render