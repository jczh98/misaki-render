#pragma once

#include "wd.h"
#include "object.h"

#define APR_FILTER_RESOLUTION 32

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT ReconstructionFilter : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)

    virtual Float eval(Float x) const;
    Float radius() const { return m_radius; }

protected:
    ReconstructionFilter(const Properties &props);
    virtual ~ReconstructionFilter();
    APR_DECLARE_CLASS()

protected:
    std::vector<Float> m_values;
    Float m_radius;
};

APR_EXTERN_CLASS(ReconstructionFilter)

} // namespace aspirin