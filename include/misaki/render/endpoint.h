#pragma once

#include "component.h"
#include "interaction.h"

namespace misaki::render {

class MSK_EXPORT Endpoint : public Component {
 public:
  Endpoint(const Properties &props);

  // Returns direction, weight
  virtual std::pair<Vector3, Vector3> sample_direction(const Vector2 &sample, const PointGeometry &geom) const;
  virtual Float pdf_direction(const PointGeometry &geom, const Vector3 &wo) const;

  // Returns sampled point geometry, weight
  virtual std::pair<PointGeometry, Vector3> sample_position(const Vector2 &sample) const;
  virtual Float pdf_position(const PointGeometry &geom) const;

  // Returns sampled point geometry, direction, weight
  virtual std::tuple<PointGeometry, Vector3, Vector3> sample_direct(const Vector2 &position_sample, const Vector2 &direction_sample) const;
  virtual Float pdf_direct(const PointGeometry &geom_surface, const PointGeometry &geom_endpoint, const Vector3 &wo) const;

  virtual Color3 eval(const PointGeometry &geom, const Vector3 &wi) const;

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