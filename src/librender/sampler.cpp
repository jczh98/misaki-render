#include <misaki/logger.h>
#include <misaki/properties.h>
#include <misaki/sampler.h>

namespace misaki {

Sampler::Sampler(const Properties &props) {
    m_sample_count = props.int_("sample_count", 1);
    m_base_seed    = props.int_("base_seed", 0);
}

Sampler::~Sampler() {}

void Sampler::seed(uint64_t seed_value) { APR_NOT_IMPLEMENTED("seed"); }

Float Sampler::next1d() { APR_NOT_IMPLEMENTED("next1d"); }

Vector2 Sampler::next2d() { APR_NOT_IMPLEMENTED("next2d"); }

APR_IMPLEMENT_CLASS(Sampler, Object, "sampler")

} // namespace misaki