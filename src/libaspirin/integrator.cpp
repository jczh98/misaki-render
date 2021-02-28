#include <aspirin/imageblock.h>
#include <aspirin/integrator.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Spectrum>
Integrator<Spectrum>::Integrator(const Properties &props) {
  m_block_size = props.get_int("block_size", MSK_BLOCK_SIZE);
}

template <typename Spectrum>
bool Integrator<Spectrum>::render(const std::shared_ptr<Scene> &scene) {
  ARP_NOT_IMPLEMENTED("render");
}


}  // namespace aspirin