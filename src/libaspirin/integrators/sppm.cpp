#include <aspirin/bsdf.h>
#include <aspirin/emitter.h>
#include <aspirin/film.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/mesh.h>
#include <aspirin/properties.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <aspirin/utils.h>
#include <fstream>
#include <array>
#include <tbb/parallel_for.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class SPPMIntegrator : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;
    using typename Base::Scene;
    using typename Base::Sensor;
    using RayDifferential = RayDifferential<Float, Spectrum>;

    SPPMIntegrator(const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) {

    }
};

} // namespace aspirin
