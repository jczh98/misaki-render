#pragma once

#include "component.h"

namespace misaki::render {

class MSK_EXPORT Sampler : public Component {
 public:
  Sampler(const Properties &props);
  virtual std::unique_ptr<Sampler> clone();
  virtual void seed(uint64_t seed_value);
  virtual Float next1d();
  virtual Vector2 next2d();
  size_t sample_count() const { return m_sample_count; }

  MSK_DECL_COMP(Component)
 protected:
  size_t m_sample_count;
  uint64_t m_base_seed;
};

}  // namespace misaki::render