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
#include <tbb/parallel_for.h>

#include <fstream>

namespace aspirin {

template <typename Float, typename Spectrum>
class DebugIntegrator final : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;

    DebugIntegrator(const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) override { return true; }

    APR_DECLARE_CLASS()
private:
};

APR_IMPLEMENT_CLASS_VARIANT(DebugIntegrator, Integrator)
APR_INTERNAL_PLUGIN(DebugIntegrator, "debug")

} // namespace aspirin