#pragma once

#include "fwd.h"
#include "object.h"

#define APR_FILTER_RESOLUTION 32

namespace aspirin {

class APR_EXPORT ReconstructionFilter : public Object {
public:
    virtual Float eval(Float x) const;
    Float radius() const { return m_radius; }

    APR_DECLARE_CLASS()
protected:
    ReconstructionFilter(const Properties &props);
    virtual ~ReconstructionFilter();

protected:
    std::vector<Float> m_values;
    Float m_radius;
};

} // namespace aspirin