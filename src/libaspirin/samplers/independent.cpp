#include <aspirin/properties.h>
#include <aspirin/sampler.h>

namespace aspirin {

class IndependentSampler final : public Sampler {
 public:
  IndependentSampler(const Properties &props = Properties()) : Sampler(props) {
    seed(PCG32_DEFAULT_STATE);
  }

  std::unique_ptr<Sampler> clone() {
    IndependentSampler *sampler = new IndependentSampler();
    sampler->m_sample_count = m_sample_count;
    return std::unique_ptr<Sampler>(sampler);
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

  Vector2 next2d() {
    return {next1d(), next1d()};
  }

  std::string to_string() const override {
    std::ostringstream oss;
    oss << "IndependentSampler[" << std::endl
        << "  sample_count = " << m_sample_count << std::endl
        << "]";
    return oss.str();
  }

  MSK_DECL_COMP(Sampler)
 private:
  std::unique_ptr<math::PCG32> m_rng;
};

MSK_EXPORT_PLUGIN(IndependentSampler)

}  // namespace aspirin