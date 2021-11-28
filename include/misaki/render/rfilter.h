#pragma once

#include "misaki/core/fwd.h"
#include "misaki/core/object.h"

#define MSK_FILTER_RESOLUTION 32

namespace misaki {

class MSK_EXPORT ReconstructionFilter : public Object {
public:
    virtual float eval(float x) const;
    inline float eval_discretized(float x) const {
        return m_values[std::min((int) std::abs(x * m_scale_factor),
                                 MSK_FILTER_RESOLUTION)];
    }
    float radius() const { return m_radius; }

    uint32_t border_size() const { return m_border_size; }

    MSK_DECLARE_CLASS()
protected:
    ReconstructionFilter(const Properties &props);
    virtual ~ReconstructionFilter();

    void init_discretization();

protected:
    std::vector<float> m_values;
    float m_radius, m_scale_factor;
    uint32_t m_border_size;
};

} // namespace misaki