#include <aspirin/emitter.h>
#include <aspirin/integrator.h>
#include <aspirin/interaction.h>
#include <aspirin/logger.h>
#include <aspirin/plugin.h>
#include <aspirin/properties.h>
#include <aspirin/ray.h>
#include <aspirin/records.h>
#include <aspirin/scene.h>
#include <aspirin/sensor.h>
#include <aspirin/shape.h>
#include <iostream>

namespace aspirin {

FileResolver *get_file_resolver() {
    static FileResolver file_resolver;
    return &file_resolver;
}

void library_nop() {}

template <typename Float, typename Spectrum>
Scene<Float, Spectrum>::Scene(const Properties &props) {
    for (auto &[name, obj] : props.objects()) {
        auto *shape      = dynamic_cast<Shape *>(obj.get());
        auto *sensor     = dynamic_cast<Sensor *>(obj.get());
        auto *integrator = dynamic_cast<Integrator *>(obj.get());
        auto *emitter    = dynamic_cast<Emitter *>(obj.get());
        if (shape) {
            if (shape->is_emitter())
                m_emitters.emplace_back(shape->emitter());
            m_bbox.expand(shape->bbox());
            m_shapes.push_back(shape);
        } else if (emitter) {
            if (!has_flag(emitter->flags(), EmitterFlags::Surface))
                m_emitters.emplace_back(emitter);
            if (emitter->is_environment()) {
                if (m_environment)
                    Throw("Can only have one environment light");
                m_environment = emitter;
            }
        } else if (sensor) {
            if (m_sensor)
                Throw("Can only have one camera.");
            m_sensor = sensor;
        } else if (integrator) {
            if (m_integrator)
                Throw("Can only have one integrator.");
            m_integrator = integrator;
        }
    }
    if (!m_integrator) {
        Log(Warn, "No integrator found! Instantiating a path tracer..");
        m_integrator = PluginManager::instance()->create_object<Integrator>(
            Properties("path"));
    }
    accel_init(props);
    for (auto emitter : m_emitters) {
        emitter->set_scene(this);
    }
}

template <typename Float, typename Spectrum> Scene<Float, Spectrum>::~Scene() {
    accel_release();
}

template <typename Float, typename Spectrum>
std::pair<typename Scene<Float, Spectrum>::DirectionSample, Spectrum>
Scene<Float, Spectrum>::sample_emitter_direction(const Interaction &ref,
                                                 const Vector2 &sample_,
                                                 bool test_visibility) const {
    DirectionSample ds;
    Spectrum spec;
    Vector2 sample(sample_);
    if (!m_emitters.empty()) {
        if (m_emitters.size() == 1) {
            std::tie(ds, spec) = m_emitters[0]->sample_direction(ref, sample);
        } else {
            auto light_sel_pdf = 1.f / m_emitters.size();
            auto index =
                std::min(uint32_t(sample.x() * (Float) m_emitters.size()),
                         (uint32_t) m_emitters.size() - 1);
            sample.x() =
                (sample.x() - index * light_sel_pdf) * m_emitters.size();
            std::tie(ds, spec) =
                m_emitters[index]->sample_direction(ref, sample);
            ds.pdf *= light_sel_pdf;
            spec *= m_emitters.size();
        }
        if (test_visibility && ds.pdf != 0.f) {
            Ray ray(ref.p, ds.d,
                    math::RayEpsilon<Float> *
                        (1.f + ref.p.cwiseAbs().maxCoeff()),
                    ds.dist * (1.f - math::ShadowEpsilon<Float>), 0);
            if (ray_test(ray))
                spec = Spectrum::Zero();
        }
    } else {
        spec = Spectrum::Zero();
    }
    return { ds, spec };
}

template <typename Float, typename Spectrum>
Float Scene<Float, Spectrum>::pdf_emitter_direction(
    const Interaction &ref, const DirectionSample &ds) const {
    if (m_emitters.size() == 1) {
        return m_emitters[0]->pdf_direction(ref, ds);
    } else {
        return reinterpret_cast<const Emitter *>(ds.object)->pdf_direction(ref,
                                                                           ds) *
               (1.f / m_emitters.size());
    }
}

/*------------------------Embree
 * specification---------------------------------*/
#if defined(APR_ENABLE_EMBREE)
#include <embree3/rtcore.h>
static RTCDevice __embree_device = nullptr;

template <typename Float, typename Spectrum>
void Scene<Float, Spectrum>::accel_init(const Properties &props) {
    if (!__embree_device)
        __embree_device = rtcNewDevice("");
    // util::Timer timer;
    RTCScene embree_scene = rtcNewScene(__embree_device);
    m_accel               = embree_scene;
    for (auto &shape : m_shapes)
        rtcAttachGeometry(embree_scene,
                          shape->embree_geometry(__embree_device));
    rtcCommitScene(embree_scene);
    // Log(Info, "Embree ready.  (took {})", util::time_string(timer.value()));
}

template <typename Float, typename Spectrum>
void Scene<Float, Spectrum>::accel_release() {
    rtcReleaseScene((RTCScene) m_accel);
}

template <typename Float, typename Spectrum>
typename Scene<Float, Spectrum>::SurfaceInteraction
Scene<Float, Spectrum>::ray_intersect(const Ray &ray) const {
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRayHit rh;
    rh.ray.org_x = ray.o.x();
    rh.ray.org_y = ray.o.y();
    rh.ray.org_z = ray.o.z();
    rh.ray.tnear = ray.mint;
    rh.ray.dir_x = ray.d.x();
    rh.ray.dir_y = ray.d.y();
    rh.ray.dir_z = ray.d.z();
    rh.ray.time  = 0;
    rh.ray.tfar  = ray.maxt;
    rh.ray.mask  = 0;
    rh.ray.id    = 0;
    rh.ray.flags = 0;
    rtcIntersect1((RTCScene) m_accel, &context, &rh);
    SurfaceInteraction si;
    if (rh.ray.tfar != ray.maxt) {
        uint32_t shape_index = rh.hit.geomID;
        uint32_t prim_index  = rh.hit.primID;

        PreliminaryIntersection pi;
        pi.shape_index = shape_index;
        pi.shape       = m_shapes[shape_index];

        pi.t          = rh.ray.tfar;
        pi.prim_index = prim_index;
        pi.prim_uv    = Vector2(rh.hit.u, rh.hit.v);

        si = pi.compute_surface_interaction(ray);
    } else {
        si.wi = -ray.d;
        si.t  = math::Infinity<Float>;
    }
    return si;
}

template <typename Float, typename Spectrum>
bool Scene<Float, Spectrum>::ray_test(const Ray &ray) const {
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    RTCRay ray2;
    ray2.org_x = ray.o.x();
    ray2.org_y = ray.o.y();
    ray2.org_z = ray.o.z();
    ray2.tnear = ray.mint;
    ray2.dir_x = ray.d.x();
    ray2.dir_y = ray.d.y();
    ray2.dir_z = ray.d.z();
    ray2.time  = 0;
    ray2.tfar  = ray.maxt;
    ray2.mask  = 0;
    ray2.id    = 0;
    ray2.flags = 0;
    rtcOccluded1((RTCScene) m_accel, &context, &ray2);
    return ray2.tfar != ray.maxt;
}

#endif

APR_IMPLEMENT_CLASS_VARIANT(Scene, Object, "scene")
APR_INSTANTIATE_CLASS(Scene)

} // namespace aspirin