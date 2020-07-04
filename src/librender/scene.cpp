#include <misaki/render/camera.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/render/light.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/ray.h>
#include <misaki/render/scene.h>
#include <misaki/render/shape.h>

namespace misaki::render {

Scene::Scene(const Properties &props) {
  for (auto &kv : props.components()) {
    auto shape = std::dynamic_pointer_cast<Shape>(kv.second);
    auto camera = std::dynamic_pointer_cast<Camera>(kv.second);
    auto integrator = std::dynamic_pointer_cast<Integrator>(kv.second);
    auto light = std::dynamic_pointer_cast<Light>(kv.second);
    if (shape) {
      if (shape->is_light()) m_lights.emplace_back(shape->light());
      m_bbox.expand(shape->bbox());
      m_shapes.push_back(shape);
    } else if (light) {
      if (!has_flag(light->flags(), LightFlags::Surface)) m_lights.emplace_back(light);
      if (light->is_environment()) {
        if (m_environment) Throw("Can only have one environment light");
        m_environment = light;
      }
    } else if (camera) {
      if (m_camera) Throw("Can only have one camera.");
      m_camera = camera;
    } else if (integrator) {
      if (m_integrator) Throw("Can only have one integrator.");
      m_integrator = integrator;
    }
  }
  if (!m_integrator) {
    Log(Warn, "No integrator found! Instantiating a path tracer..");
    m_integrator = PluginManager::instance()->create_comp<Integrator>(Properties("path"));
  }
  accel_init(props);
  for (auto light : m_lights) {
    light->set_scene(this);
  }
}

Scene::~Scene() {
  accel_release();
}

std::pair<DirectSample, Color3> Scene::sample_direct_light(const PointGeometry &geom,
                                                           const Vector2 &sample_, bool test_visibility) const {
  DirectSample ds;
  Color3 emitted;
  Vector2 sample(sample_);
  if (!m_lights.empty()) {
    if (m_lights.size() == 1) {
      std::tie(ds, emitted) = m_lights[0]->sample_direct(geom, sample);
    } else {
      auto light_sel_pdf = 1.f / m_lights.size();
      auto index = std::min(uint32_t(sample.x() * (Float)m_lights.size()), (uint32_t)m_lights.size() - 1);
      sample.x() = (sample.x() - index * light_sel_pdf) * m_lights.size();
      std::tie(ds, emitted) = m_lights[index]->sample_direct(geom, sample);
      ds.pdf *= light_sel_pdf;
      emitted *= m_lights.size();
    }
    if (test_visibility && ds.pdf != 0.f) {
      Ray ray(geom.p, ds.d, RayEpsilon<Float> * (1.f + math::hmax(math::abs(geom.p))),
              ds.dist * (1.f - ShadowEpsilon<Float>), 0);
      if (ray_test(ray)) emitted = 0.f;
    }
  } else {
    emitted = 0.f;
  }
  return {ds, emitted};
}

Float Scene::pdf_direct_light(const PointGeometry &geom_ref, const DirectSample &ds, const Light *light) const {
  if (m_lights.size() == 1) {
    return m_lights[0]->pdf_direct(geom_ref, ds);
  } else {
    return light->pdf_direct(geom_ref, ds) *
           (1.f / m_lights.size());
  }
}

/*------------------------Embree specification---------------------------------*/
#if defined(MSK_ENABLE_EMBREE)
#include <embree3/rtcore.h>
static RTCDevice __embree_device = nullptr;

void Scene::accel_init(const Properties &props) {
  if (!__embree_device)
    __embree_device = rtcNewDevice("");
  util::Timer timer;
  RTCScene embree_scene = rtcNewScene(__embree_device);
  m_accel = embree_scene;
  for (auto &shape : m_shapes)
    rtcAttachGeometry(embree_scene, shape->embree_geometry(__embree_device));
  rtcCommitScene(embree_scene);
  Log(Info, "Embree ready.  (took {})", util::time_string(timer.value()));
}

void Scene::accel_release() {
  rtcReleaseScene((RTCScene)m_accel);
}

std::optional<SceneInteraction> Scene::ray_intersect(const Ray &ray) const {
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
  rh.ray.time = 0;
  rh.ray.tfar = ray.maxt;
  rh.ray.mask = 0;
  rh.ray.id = 0;
  rh.ray.flags = 0;
  rtcIntersect1((RTCScene)m_accel, &context, &rh);
  if (rh.ray.tfar != ray.maxt) {
    uint32_t shape_index = rh.hit.geomID;
    uint32_t prim_index = rh.hit.primID;
    auto p = m_shapes[shape_index]->compute_surface_point(prim_index, {rh.hit.u, rh.hit.v});
    return SceneInteraction::make_surface_interaction(
        PointGeometry::make_on_surface(p.p, p.ng, p.ns, p.uv),
        m_shapes[shape_index].get());
  } else {
    return {};
  }
}

bool Scene::ray_test(const Ray &ray) const {
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
  ray2.time = 0;
  ray2.tfar = ray.maxt;
  ray2.mask = 0;
  ray2.id = 0;
  ray2.flags = 0;
  rtcOccluded1((RTCScene)m_accel, &context, &ray2);
  return ray2.tfar != ray.maxt;
}

#endif

MSK_REGISTER_CLASS(Scene)

}  // namespace misaki::render