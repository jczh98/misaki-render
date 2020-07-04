#pragma once

#include "component.h"
#if defined(MSK_ENABLE_EMBREE)
#include <embree3/rtcore.h>
#endif

namespace misaki::render {

class MSK_EXPORT Shape : public Component {
 public:
  Shape(const Properties &props);

  // Returns sampled point geometry and associated pdf
  virtual std::pair<PointGeometry, Float> sample_position(const Vector2 &sample) const;
  virtual Float pdf_position(const PointGeometry &geom) const;

  struct InterpolatedPoint {
    Vector3 p;  // position
    Vector3 ns;
    Vector3 ng;
    Vector2 uv;
  };

  virtual InterpolatedPoint compute_surface_point(int prim_index, const Vector2 &uv) const;

  bool is_mesh() const { return m_mesh; }

  const BSDF *bsdf() const { return m_bsdf.get(); }
  BSDF *bsdf() { return m_bsdf.get(); }

  bool is_light() const { return (bool)m_light; }
  const Light *light() const { return m_light.get(); }
  Light *light() { return m_light.get(); }

  virtual BoundingBox3 bbox() const;
  virtual BoundingBox3 bbox(uint32_t index) const;
  virtual Float surface_area() const;

#if defined(MSK_ENABLE_EMBREE)
  virtual RTCGeometry embree_geometry(RTCDevice device) const;
#endif

  MSK_DECL_COMP(Component)
 protected:
  std::shared_ptr<Light> m_light;
  std::shared_ptr<BSDF> m_bsdf;
  Transform4 m_world_transform;
  bool m_mesh = false;
  std::string m_id;
};

}  // namespace misaki::render