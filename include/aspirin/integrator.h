#pragma once

#include "fwd.h"
#include "object.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Integrator : public Object {
public:
    using Scene  = Scene<Float, Spectrum>;
    using Sensor = Sensor<Float, Spectrum>;

    virtual bool render(Scene *scene, Sensor *sensor);

    APR_DECLARE_CLASS()
protected:
    Integrator(const Properties &props);
    virtual ~Integrator();
protected:
    uint32_t m_block_size;
};

APR_EXTERN_CLASS(Integrator)

} // namespace aspirin