#include <misaki/render/endpoint.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Endpoint::Endpoint(const Properties &props) : m_id(props.id()) {
  m_world_transform = props.transform("to_world", Transform4());
}

MSK_REGISTER_CLASS(Endpoint)

}  // namespace misaki::render