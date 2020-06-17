#pragma once

#include "component.h"

namespace misaki::render {

class Sampler : public Component {
 public:
  virtual std::shared_ptr<Sampler> clone() = 0;
  virtual void seed(uint64_t seed_value);
  virtual Float next1d();
  virtual Vector2 next2d();
  size_t sample_count() const { return m_sample_count; }
  MSK_DECL_COMP(Component)
 protected:
  size_t m_sample_count;
};
}  // namespace misaki::render