#pragma once

#include "component.h"
#include "fwd.h"

#define MSK_FILTER_RESOLUTION 32

namespace misaki::render {

class MSK_EXPORT ReconstructionFilter : public Component {
 public:
  ReconstructionFilter(const Properties &props);
  virtual Float eval(Float x) const;
  Float radius() const { return m_radius; }
  MSK_DECL_COMP(Component)
 protected:
  std::vector<Float> m_values;
  Float m_radius;
};

}  // namespace misaki::render