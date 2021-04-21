#include <array>
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
#include <tbb/parallel_for.h>

namespace aspirin {

template <typename Float, typename Spectrum>
class SPPMIntegrator : public Integrator<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Integrator<Float, Spectrum>;
    using typename Base::Scene;
    using typename Base::Sensor;
    using BSDF            = BSDF<Float, Spectrum>;
    using RayDifferential = RayDifferential<Float, Spectrum>;

    struct SPPMPixel {

        Float radius = 0;
        Spectrum value;

        struct VisiblePoint {
            VisiblePoint() {}
            VisiblePoint(const Vector3 &p, const Vector3 &wi, const BSDF *bsdf,
                         const Spectrum &beta)
                : p(p), wi(wi), bsdf(bsdf), beta(beta) {}
            Vector3 p;
            Vector3 wi;
            const BSDF *bsdf = nullptr;
            Spectrum beta;
        };

        void atomic_add_phi(uint32_t idx, Float v) {
            auto current = phi[idx].load();
            while (!phi[idx].compare_exchange_weak(current, current + v))
                ;
        }
        std::atomic<Float> phi[3];
        std::atomic<int> m;
        float n = 0;
        Spectrum tau;
    };

    struct SPPMPixelListNode {
        SPPMPixel *pixel;
        SPPMPixelListNode *next;
    };

    inline unsigned int hash(const Vector3i &p, int hash_size) {
        return (unsigned int)((p.x * 73856093) ^ (p.y * 19349663) ^
                              (p.z * 83492791)) %
            hash_size;
    }

    SPPMIntegrator(const Properties &props) : Base(props) {}

    bool render(Scene *scene, Sensor *sensor) {}

    Float m_initial_radius  = 1.f;
    int m_iterations        = 64;
    int m_max_depth         = 5;
    int m_photons           = 100;
    int m_develop_frequency = 1 << 31;
};

} // namespace aspirin
