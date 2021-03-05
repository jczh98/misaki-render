#include <aspirin/properties.h>
#include <aspirin/rfilter.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class GaussianFilter final : public ReconstructionFilter<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = ReconstructionFilter<Float, Spectrum>;
    using Base::m_radius;

    GaussianFilter(const Properties &props) : Base(props) {
        m_stddev = props.get_float("stddev", 0.5f);
        m_radius = 4 * m_stddev;
        m_alpha  = -1.f / (2.f * m_stddev * m_stddev);
        m_bias   = std::exp(m_alpha * m_radius * m_radius);
    }

    Float eval(Float x) const override {
        return std::max(0.f, std::exp(m_alpha * x * x) - m_bias);
    }

    APR_DECLARE_CLASS()
private:
    Float m_stddev, m_alpha, m_bias;
};

APR_IMPLEMENT_CLASS_VARIANT(GaussianFilter, ReconstructionFilter)
APR_INTERNAL_PLUGIN(GaussianFilter, "gaussian")

} // namespace aspirin