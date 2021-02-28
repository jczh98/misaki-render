#include <aspirin/light.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Spectrum>
Light<Spectrum>::Light(const Properties &props) : Endpoint<Spectrum>(props) {
}

}  // namespace aspirin