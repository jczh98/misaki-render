#include <misaki/render/imageblock.h>
#include <misaki/render/integrator.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>

namespace misaki::render {

Integrator::Integrator(const Properties &props) {
  m_block_size = props.get_int("block_size", MSK_BLOCK_SIZE);
}

bool Integrator::render(const std::shared_ptr<Scene> &scene) {
  MSK_NOT_IMPLEMENTED("render");
}

MSK_REGISTER_CLASS(Integrator)

}  // namespace misaki::render