#include <aspirin/imageblock.h>
#include <aspirin/integrator.h>
#include <aspirin/logger.h>
#include <aspirin/properties.h>

namespace aspirin {

template <typename Float, typename Spectrum>
Integrator<Float, Spectrum>::Integrator(const Properties &props) {
    m_block_size = props.get_int("block_size", APR_BLOCK_SIZE);
}

template <typename Float, typename Spectrum>
Integrator<Float, Spectrum>::~Integrator() {}

template <typename Float, typename Spectrum>
bool Integrator<Float, Spectrum>::render(Scene *scene, Sensor *sensor) {
    APR_NOT_IMPLEMENTED("render");
}

APR_IMPLEMENT_CLASS_VARIANT(Integrator, Object, "integrator")
APR_INSTANTIATE_CLASS(Integrator)

} // namespace aspirin