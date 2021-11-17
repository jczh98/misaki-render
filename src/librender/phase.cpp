#include <misaki/render/phase.h>
#include <misaki/core/properties.h>

namespace misaki {

PhaseFunction::PhaseFunction(const Properties &props)
    : m_flags(+PhaseFunctionFlags::None), m_id(props.id()) {}

PhaseFunction::~PhaseFunction() {}

MSK_IMPLEMENT_CLASS(PhaseFunction, Object, "phase")

} // namespace misaki