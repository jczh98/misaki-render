#include <aspirin/bsdf.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Float, typename Spectrum>
BSDF<Float, Spectrum>::BSDF(const Properties &props)
    : m_flags(+BSDFFlags::None), m_id(props.id()) {}

APR_IMPLEMENT_CLASS_VARIANT(BSDF, Object, "bsdf")
APR_INSTANTIATE_CLASS(BSDF)

} // namespace aspirin