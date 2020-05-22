#pragma once

#include "fwd.h"
#include "component.h"

namespace misaki::render {

class MSK_EXPORT Camera : public Component {
 public:
  Camera(const Properties &);

  MSK_DECL_COMP(Component)
};

}