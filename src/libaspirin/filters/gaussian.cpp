#include <aspirin/properties.h>
#include <aspirin/rfilter.h>

namespace aspirin {

class GaussianFilter final : public ReconstructionFilter {
public:

    GaussianFilter(const Properties &props) : ReconstructionFilter(props) {
        m_stddev = props.float_("stddev", 0.5f);
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

APR_IMPLEMENT_CLASS(GaussianFilter, ReconstructionFilter)
APR_INTERNAL_PLUGIN(GaussianFilter, "gaussian")

} // namespace aspirin