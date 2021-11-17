#include <misaki/render/imageblock.h>
#include <misaki/render/integrator.h>
#include <misaki/core/logger.h>
#include <misaki/core/properties.h>

namespace misaki {

Integrator::Integrator(const Properties &props) {
    m_block_size = props.int_("block_size", APR_BLOCK_SIZE);
}

Integrator::~Integrator() {}

bool Integrator::render(Scene *scene, Sensor *sensor) {
    MSK_NOT_IMPLEMENTED("render");
}

MSK_IMPLEMENT_CLASS(Integrator, Object, "integrator")

} // namespace misaki