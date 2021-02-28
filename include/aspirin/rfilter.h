#pragma once

#include "component.h"
#include "fwd.h"

#define MSK_FILTER_RESOLUTION 32

namespace aspirin {

template <typename Spectrum>
class APR_EXPORT ReconstructionFilter : public Component {
 public:
  ReconstructionFilter(const Properties &props);
  virtual Float eval(Float x) const;
  Float radius() const { return m_radius; }

 protected:
  std::vector<Float> m_values;
  Float m_radius;
};

}  // namespace aspirin