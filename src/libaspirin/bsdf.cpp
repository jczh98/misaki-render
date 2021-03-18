#include <aspirin/bsdf.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Float, typename Spectrum>
BSDF<Float, Spectrum>::BSDF(const Properties &props)
    : m_flags(+BSDFFlags::None), m_id(props.id()) {}

template <typename Float, typename Spectrum> BSDF<Float, Spectrum>::~BSDF() {}

template <typename Float, typename Spectrum>
Spectrum BSDF<Float, Spectrum>::eval_null_transmission(
    const SurfaceInteraction &) const {
    return Spectrum::Zero();
}

template <typename Float, typename Spectrum>
std::string BSDF<Float, Spectrum>::id() const {
    return m_id;
}

APR_IMPLEMENT_CLASS_VARIANT(BSDF, Object, "bsdf")
APR_INSTANTIATE_CLASS(BSDF)

} // namespace aspirin