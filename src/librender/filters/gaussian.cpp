#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/rfilter.h>

namespace misaki {

class GaussianFilter final : public ReconstructionFilter {
public:
    GaussianFilter(const Properties &props) : ReconstructionFilter(props) {
        m_stddev = props.float_("stddev", 0.5f);
        m_radius = 4 * m_stddev;
        m_alpha  = -1.f / (2.f * m_stddev * m_stddev);
        m_bias   = std::exp(m_alpha * m_radius * m_radius);

        init_discretization();
    }

    float eval(float x) const override {
        return std::max(0.f, std::exp(m_alpha * x * x) - m_bias);
    }

    std::string to_string() const override {
        return fmt::format("GaussianFilter[stddev={}, radius={}]", m_stddev,
                           m_radius);
    }

    MSK_DECLARE_CLASS()
private:
    float m_stddev, m_alpha, m_bias;
};

MSK_IMPLEMENT_CLASS(GaussianFilter, ReconstructionFilter)
MSK_REGISTER_INSTANCE(GaussianFilter, "gaussian")

} // namespace misaki