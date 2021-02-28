#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/sampler.h>

namespace aspirin {

template <typename Spectrum>
Sampler<Spectrum>::Sampler(const Properties &props) {
  m_sample_count = props.get_int("sample_count", 1);
  m_base_seed = props.get_int("base_seed", 0);
}

template <typename Spectrum>
std::unique_ptr<Sampler> Sampler<Spectrum>::clone() {
  ARP_NOT_IMPLEMENTED("clone");
}

template <typename Spectrum>
void Sampler<Spectrum>::seed(uint64_t seed_value) {
  ARP_NOT_IMPLEMENTED("seed");
}

template <typename Spectrum>
Float Sampler<Spectrum>::next1d() {
  ARP_NOT_IMPLEMENTED("next1d");
}

template <typename Spectrum>
Vector2 Sampler<Spectrum>::next2d() {
  ARP_NOT_IMPLEMENTED("next2d");
}

}  // namespace aspirin