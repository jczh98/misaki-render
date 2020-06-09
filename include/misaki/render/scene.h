#pragma once

#include "component.h"
#include "fwd.h"

namespace misaki::render {

class Scene : public Component {
 public:
  Scene(const Properties &props);
  MSK_DECL_COMP(Component)
};

}  // namespace misaki::render