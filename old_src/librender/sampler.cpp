#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/sampler.h>

namespace misaki::render {

Sampler::Sampler(const Properties &props) {
  m_sample_count = props.get_int("sample_count", 1);
  m_base_seed = props.get_int("base_seed", 0);
}

std::unique_ptr<Sampler> Sampler::clone() {
  MSK_NOT_IMPLEMENTED("clone");
}

void Sampler::seed(uint64_t seed_value) {
  MSK_NOT_IMPLEMENTED("seed");
}

Float Sampler::next1d() {
  MSK_NOT_IMPLEMENTED("next1d");
}

Vector2 Sampler::next2d() {
  MSK_NOT_IMPLEMENTED("next2d");
}

MSK_REGISTER_CLASS(Sampler)

}  // namespace misaki::render