#pragma once

#include "component.h"

namespace aspirin {

template <typename Spectrum>
class APR_EXPORT Sampler : public Component {
 public:
  Sampler(const Properties &props);
  virtual std::unique_ptr<Sampler> clone();
  virtual void seed(uint64_t seed_value);
  virtual Float next1d();
  virtual Vector2 next2d();
  size_t sample_count() const { return m_sample_count; }

 protected:
  size_t m_sample_count;
  uint64_t m_base_seed;
};

}  // namespace aspirin