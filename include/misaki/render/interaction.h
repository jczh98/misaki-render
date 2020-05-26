#pragma once

#include "fwd.h"

namespace misaki::render {

struct PointGeometry {
  Vector3 p;   // Position
  Vector2 uv;  // Texture coordinates
};

struct SceneInteraction {
  enum Type : uint32_t {
    None = 0,
    CameraEndpoint,
    LightEndpoint,
    SurfaceInteraction,
    MediumInteraction,
    Endpoint,
    Intermediate
  } int type = None;
  PointGeometry geom;  // Interaction point information

  bool is_type(uint32_t type_flag) const {
    return (type & type_flag) > 0;
  }

  static SceneInteraction make_surface_interaction(const PointGeometry &geom) {
    SceneInteraction si;
    si.type = SurfaceInteraction;
    si.geom = geom;
    return si;
  }
};

}  // namespace misaki::render