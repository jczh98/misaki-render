#include <misaki/emitter.h>
#include <misaki/properties.h>

namespace misaki {

Emitter::Emitter(const Properties &props) : Endpoint(props) {}

Emitter::~Emitter() {}

APR_IMPLEMENT_CLASS(Emitter, Endpoint, "emitter")

} // namespace misaki