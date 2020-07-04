#pragma once

#include <optional>

#include "component.h"
#include "interaction.h"

namespace misaki::render {

class MSK_EXPORT Endpoint : public Component {
 public:
  Endpoint(const Properties &props);

  // Returns Sampled ray with structred RaySample
  virtual std::pair<Ray, Vector3> sample_ray(const Vector2 &pos_sample) const;

  // Returns direction, weight
  virtual std::pair<Vector3, Vector3> sample_direction(const Vector2 &sample, const PointGeometry &geom) const;
  virtual Float pdf_direction(const PointGeometry &geom, const Vector3 &wo) const;

  // Returns sampled point geometry, weight
  virtual std::pair<PointGeometry, Vector3> sample_position(const Vector2 &sample) const;
  virtual Float pdf_position(const PointGeometry &geom) const;

  // Returns sampled point geometry, direction, radiance, pdf
  virtual std::pair<DirectSample, Color3> sample_direct(const PointGeometry &geom_ref, const Vector2 &position_sample) const;
  // Returns pdf associated with area measure
  virtual Float pdf_direct(const PointGeometry &geom_ref, const DirectSample &ds) const;

  virtual Color3 eval(const SceneInteraction &si) const;

  virtual void set_shape(Shape *shape);
  virtual void set_scene(const Scene *scene);

  Shape *shape() { return m_shape; }
  const Shape *shape() const { return m_shape; }

  MSK_DECL_COMP(Component)
 protected:
  Transform4 m_world_transform;
  Shape *m_shape = nullptr;
  std::string m_id;
};

}  // namespace misaki::render