#include <iostream>
#include <misaki/core/logger.h>
#include <misaki/core/properties.h>
#include <misaki/render/rfilter.h>

namespace misaki {

ReconstructionFilter::ReconstructionFilter(const Properties &props) {}

ReconstructionFilter::~ReconstructionFilter() {}

void ReconstructionFilter::init_discretization() {
    m_values.resize(MSK_FILTER_RESOLUTION + 1);
    float sum = 0.f;
    for (size_t i = 0; i < MSK_FILTER_RESOLUTION; ++i) {
        m_values[i] = eval(float(m_radius * i) / MSK_FILTER_RESOLUTION);
        sum += m_values[i];
    }

    m_values[MSK_FILTER_RESOLUTION] = 0;
    m_scale_factor                  = float(MSK_FILTER_RESOLUTION) / m_radius;
    m_border_size                   = (int) std::ceil(m_radius - .5f);
    sum *= 2 * m_radius / MSK_FILTER_RESOLUTION;
    float normalization = 1.0f / sum;
    for (size_t i = 0; i < MSK_FILTER_RESOLUTION; ++i)
        m_values[i] *= normalization;
}

float ReconstructionFilter::eval(float x) const { MSK_NOT_IMPLEMENTED("eval"); }

MSK_IMPLEMENT_CLASS(ReconstructionFilter, Object, "rfilter")

} // namespace misaki