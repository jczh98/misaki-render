#include <aspirin/logger.h>
#include <aspirin/properties.h>
#include <aspirin/sampler.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Sampler<Float, Spectrum>::Sampler(const Properties &props) {
    m_sample_count = props.get_int("sample_count", 1);
    m_base_seed    = props.get_int("base_seed", 0);
}

template <typename Float, typename Spectrum>
Sampler<Float, Spectrum>::~Sampler() {}

template <typename Float, typename Spectrum>
void Sampler<Float, Spectrum>::seed(uint64_t seed_value) {
    APR_NOT_IMPLEMENTED("seed");
}

template <typename Float, typename Spectrum>
Float Sampler<Float, Spectrum>::next1d() {
    APR_NOT_IMPLEMENTED("next1d");
}

template <typename Float, typename Spectrum>
typename Sampler<Float, Spectrum>::Vector2 Sampler<Float, Spectrum>::next2d() {
    APR_NOT_IMPLEMENTED("next2d");
}

APR_IMPLEMENT_CLASS_VARIANT(Sampler, Object, "sampler")
APR_INSTANTIATE_CLASS(Sampler)

} // namespace aspirin