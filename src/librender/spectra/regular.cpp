#include "misaki/core/fwd.h"
#include "misaki/core/object.h"
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/render/interaction.h>
#include <misaki/render/srgb.h>
#include <misaki/render/texture.h>

namespace misaki {

class SpectrumContinuousDistribution {

public:
    SpectrumContinuousDistribution() = default;

    SpectrumContinuousDistribution(const Eigen::Vector2f &range,
                                   const float *values, size_t size)
        : m_range(range) {
        m_pdf.resize(size);
        for (int i = 0; i < size; i++) {
            m_pdf[i] = values[i];
        }
        update();
    }

    void update() {
        size_t size = m_pdf.size();
        if (size < 2)
            Throw("ContinuousDistribution: needs at least two entries!");

        if (!(m_range.x() < m_range.y()))
            Throw("ContinuousDistribution: invalid range!");

        if (m_cdf.size() != size - 1)
            m_cdf.resize(size - 1);

        double range         = double(m_range.y()) - double(m_range.x()),
               interval_size = range / (size - 1), integral = 0.;

        float *pdf_ptr = m_pdf.data(), *cdf_ptr = m_cdf.data();

        m_valid = Eigen::Vector2i::Constant((uint32_t) -1);

        for (size_t i = 0; i < size - 1; i++) {
            double y0 = (double) pdf_ptr[0], y1 = (double) pdf_ptr[1];

            double value = 0.5 * interval_size * (y0 + y1);

            integral += value;
            *cdf_ptr++ = integral;
            pdf_ptr++;

            if (y0 < 0. || y1 < 0.) {
                Throw("ContinuousDistribution: entries must be non-negative!");
            } else if (value > 0.) {
                // Determine the first and last wavelength bin with nonzero
                // density
                if (m_valid.x() == (uint32_t) -1)
                    m_valid.x() = (uint32_t) i;
                m_valid.y() = (uint32_t) i;
            }
        }

        if (m_valid.x() == (uint32_t) -1 || m_valid.y() == (uint32_t) -1)
            Throw("ContinuousDistribution: no probability mass found!");
        m_integral          = integral;
        m_normalization     = 1. / integral;
        m_interval_size     = interval_size;
        m_inv_interval_size = 1. / interval_size;
    }

    Wavelength eval_pdf(Wavelength x) const {
        x = (x - m_range.x()) * m_inv_interval_size;

        using Index = Eigen::Array<uint32_t, Wavelength::SizeAtCompileTime, 1>;

        Index index =
            x.cast<uint32_t>().cwiseMin(uint32_t(m_pdf.size() - 2)).cwiseMax(0);

        Wavelength y0, y1;
        for (int i = 0; i < index.size(); i++) {
            y0.coeffRef(i) = m_pdf[index.coeff(i)];
            y1.coeffRef(i) = m_pdf[index.coeff(i) + 1];
        }

        Wavelength w1 = x - index.cast<float>(), w0 = 1.f - w1;

        return w0 * y0 + w1 * y1;
    }

    float integral() const { return m_integral; }

private:
    std::vector<float> m_pdf, m_cdf;
    float m_integral          = 0.f;
    float m_normalization     = 0.f;
    float m_interval_size     = 0.f;
    float m_inv_interval_size = 0.f;
    Eigen::Vector2f m_range   = { 0, 0 };
    Eigen::Vector2i m_valid;
};

std::ostream &operator<<(std::ostream &os,
                         const SpectrumContinuousDistribution &distr) {
    os << "SpectrumContinuousDistribution[" << std::endl
       << "  integral = " << distr.integral() << "," << std::endl
       << "]";
    return os;
}

class MSK_EXPORT RegularSpectrum : public Texture {
public:
    RegularSpectrum(const Properties &props)
        : Texture(props) {
        Eigen::Vector2f wavelength_range(props.float_("lambda_min"),
                                         props.float_("lambda_max"));

        size_t size = props.int_("size");

        const float *values = (float *) props.pointer("values");

        m_distr =
            SpectrumContinuousDistribution(wavelength_range, values, size);
    }

    Spectrum eval(const SceneInteraction &si) const override {
        return m_distr.eval_pdf(si.wavelengths);
    }

    float mean() const override { return m_distr.integral(); }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "SRGBReflectanceSpectrum[" << std::endl
            << "  distr  = " << m_distr << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
private:
    SpectrumContinuousDistribution m_distr;
};

MSK_IMPLEMENT_CLASS(RegularSpectrum, Texture)
MSK_REGISTER_INSTANCE(RegularSpectrum, "regular")

} // namespace misaki
