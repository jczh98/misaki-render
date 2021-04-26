#include <aspirin/imageblock.h>
#include <aspirin/integrator.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

Integrator::Integrator(const Properties &props) {
    m_block_size = props.get_int("block_size", APR_BLOCK_SIZE);
}

Integrator::~Integrator() {}

bool Integrator::render(Scene *scene, Sensor *sensor) {
    APR_NOT_IMPLEMENTED("render");
}

APR_IMPLEMENT_CLASS(Integrator, Object, "integrator")

} // namespace aspirin