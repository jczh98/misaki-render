#include <aspirin/emitter.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Emitter<Float, Spectrum>::Emitter(const Properties &props)
    : Endpoint<Float, Spectrum>(props) {}

template <typename Float, typename Spectrum>
Emitter<Float, Spectrum>::~Emitter() {}

APR_IMPLEMENT_CLASS_VARIANT(Emitter, Endpoint, "emitter")
APR_INSTANTIATE_CLASS(Emitter)

} // namespace aspirin