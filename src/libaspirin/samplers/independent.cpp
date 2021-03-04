#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/sampler.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class IndependentSampler final : public Sampler<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Sampler<Float, Spectrum>::m_sample_count;
    using Sampler<Float, Spectrum>::m_base_seed;

    IndependentSampler(const Properties &props = Properties())
        : Sampler<Float, Spectrum>(props) {
        seed(PCG32_DEFAULT_STATE);
    }

    ref<Sampler<Float, Spectrum>> clone() override {
        IndependentSampler *sampler = new IndependentSampler();
        sampler->m_sample_count     = m_sample_count;
        return sampler;
    }

    void seed(uint64_t seed_value) {
        if (!m_rng)
            m_rng = std::make_unique<math::PCG32>();

        seed_value += m_base_seed;
        m_rng->seed(seed_value, PCG32_DEFAULT_STREAM);
    }

    Float next1d() {
        if constexpr (std::is_same_v<Float, float>)
            return m_rng->next_float32();
        else
            return m_rng->next_float64();
    }

    Vector2 next2d() { return { next1d(), next1d() }; }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "IndependentSampler[" << std::endl
            << "  sample_count = " << m_sample_count << std::endl
            << "]";
        return oss.str();
    }

    APR_DECLARE_CLASS()
private:
    std::unique_ptr<math::PCG32> m_rng;
};

APR_IMPLEMENT_CLASS_VARIANT(IndependentSampler, Sampler)
APR_INTERNAL_PLUGIN(IndependentSampler, "independent")

} // namespace aspirin