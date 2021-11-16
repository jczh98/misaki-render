#pragma once

#include "fwd.h"
#include "object.h"

#define APR_FILTER_RESOLUTION 32

namespace misaki {

class MSK_EXPORT ReconstructionFilter : public Object {
public:
    virtual float eval(float x) const;
    float radius() const { return m_radius; }

    MSK_DECLARE_CLASS()
protected:
    ReconstructionFilter(const Properties &props);
    virtual ~ReconstructionFilter();

protected:
    std::vector<float> m_values;
    float m_radius;
};

} // namespace misaki