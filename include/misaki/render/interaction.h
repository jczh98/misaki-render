#pragma once

#include "fwd.h"

namespace misaki::render {

struct PointGeometry {
  Vector3 p;        // Position
  Vector2 uv;       // Texture coordinates
  Frame sh_frame = Frame({0, 0, 0});   // Shading frame with shading normal
  Frame geo_frame = Frame({0, 0, 0});  // Geo frame with geo normal

  static PointGeometry make_on_surface(const Vector3 &p, const Vector3 &ng,
                                       const Vector3 &ns, const Vector2 &uv) {
    PointGeometry geom;
    geom.p = p;
    geom.geo_frame = Frame(ng);
    geom.sh_frame = Frame(ns);
    geom.uv = uv;
    return geom;
  }
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
  };
  int type = None;
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