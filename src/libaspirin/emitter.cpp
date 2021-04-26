#include <aspirin/emitter.h>
#include <aspirin/properties.h>

namespace aspirin {

Emitter::Emitter(const Properties &props) : Endpoint(props) {}

Emitter::~Emitter() {}

APR_IMPLEMENT_CLASS(Emitter, Endpoint, "emitter")

} // namespace aspirin