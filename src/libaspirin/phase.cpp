#include <aspirin/phase.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Float, typename Spectrum>
PhaseFunction<Float, Spectrum>::PhaseFunction(const Properties &props)
    : m_flags(+PhaseFunctionFlags::None), m_id(props.id()) {}

template <typename Float, typename Spectrum>
PhaseFunction<Float, Spectrum>::~PhaseFunction() {}

APR_IMPLEMENT_CLASS_VARIANT(PhaseFunction, Object, "phase")
APR_INSTANTIATE_CLASS(PhaseFunction)

} // namespace aspirin