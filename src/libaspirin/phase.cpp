#include <aspirin/phase.h>
#include <aspirin/properties.h>

namespace aspirin {

PhaseFunction::PhaseFunction(const Properties &props)
    : m_flags(+PhaseFunctionFlags::None), m_id(props.id()) {}

PhaseFunction::~PhaseFunction() {}

APR_IMPLEMENT_CLASS(PhaseFunction, Object, "phase")

} // namespace aspirin