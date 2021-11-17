#include <misaki/render/emitter.h>
#include <misaki/core/properties.h>

namespace misaki {

Emitter::Emitter(const Properties &props) : Endpoint(props) {}

Emitter::~Emitter() {}

MSK_IMPLEMENT_CLASS(Emitter, Endpoint, "emitter")

} // namespace misaki