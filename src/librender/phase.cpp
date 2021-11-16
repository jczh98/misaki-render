#include <misaki/phase.h>
#include <misaki/properties.h>

namespace misaki {

PhaseFunction::PhaseFunction(const Properties &props)
    : m_flags(+PhaseFunctionFlags::None), m_id(props.id()) {}

PhaseFunction::~PhaseFunction() {}

APR_IMPLEMENT_CLASS(PhaseFunction, Object, "phase")

} // namespace misaki