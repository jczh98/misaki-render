#include <misaki/render/light.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Light::Light(const Properties &props) : Endpoint(props) {
}

MSK_REGISTER_CLASS(Light)

}  // namespace misaki::render