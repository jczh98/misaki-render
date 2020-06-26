#pragma once

#include "component.h"
#if defined(MSK_ENABLE_EMBREE)
#include <embree3/rtcore.h>
#endif

namespace misaki::render {

class MSK_EXPORT Shape : public Component {
 public:
  Shape(const Properties &props);

  struct InterpolatedPoint {
    Vector3 p; // position
    Vector3 ns;
    Vector3 ng;
    Vector2 uv;
  };

  virtual InterpolatedPoint compute_surface_point(int prim_index, const Vector2 &uv) const;
  
  bool is_mesh() const { return m_mesh; }

  virtual BoundingBox3 bbox() const;
  virtual BoundingBox3 bbox(uint32_t index) const;
  virtual Float surface_area() const;

#if defined(MSK_ENABLE_EMBREE)
  virtual RTCGeometry embree_geometry(RTCDevice device) const;
#endif

  MSK_DECL_COMP(Component)
 protected:
  bool m_mesh = false;
  std::string m_id;
};

}  // namespace misaki::render