#pragma once

#include "component.h"
#include "fwd.h"

namespace misaki::render {

class Scene : public Component {
 public:
  Scene(const Properties& props);

  const Camera* camera() const { return m_camera.get(); }
  Camera* camera() { return m_camera.get(); }

  const Integrator* integrator() const { return m_integrator.get(); }
  Integrator* integrator() { return m_integrator.get(); }

  MSK_DECL_COMP(Component)
 protected:
  std::shared_ptr<Integrator> m_integrator;
  std::shared_ptr<Camera> m_camera;
};

}  // namespace misaki::render