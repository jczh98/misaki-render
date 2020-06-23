#pragma once

#include <optional>

#include "component.h"
#include "fwd.h"
#include "ray.h"

namespace misaki::render {

class Scene : public Component {
 public:
  Scene(const Properties& props);
  ~Scene();

  bool ray_test(const Ray& ray) const;
  std::optional<SceneInteraction> ray_intersect(const Ray& ray) const;
  void accel_init(const Properties& props);
  void accel_release();

  const Camera* camera() const { return m_camera.get(); }
  Camera* camera() { return m_camera.get(); }

  const Integrator* integrator() const { return m_integrator.get(); }
  Integrator* integrator() { return m_integrator.get(); }

  const BoundingBox3& bbox() const { return m_bbox; }

  MSK_DECL_COMP(Component)
 protected:
  void* m_accel = nullptr;
  std::shared_ptr<Integrator> m_integrator;
  std::shared_ptr<Camera> m_camera;
  std::vector<std::shared_ptr<Shape>> m_shapes;
  BoundingBox3 m_bbox;
};

}  // namespace misaki::render