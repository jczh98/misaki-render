#pragma once

#include "fwd.h"

namespace misaki::render {

struct PointGeometry {
  bool degenerated;                  // True if surface is a point
  bool infinite;                     // True if point is at infinity
  Vector3 p{0.f};                    // Position
  Vector2 uv{0.f};                   // Texture coordinates
  Vector3 n{0.f};                    // Geometric normal
  Vector3 wo{0.f};                   // Direction from a point at infinity (only when infinite = true)
  Frame shading = Frame({0, 0, 0});  // Shading frame with shading normal

  static PointGeometry make_degenerated(const Vector3 &p) {
    PointGeometry geom;
    geom.degenerated = true;
    geom.infinite = false;
    geom.p = p;
    return geom;
  }

  static PointGeometry make_infinite(const Vector3 &p, const Vector3 &wo) {
    PointGeometry geom;
    geom.p = p;
    geom.degenerated = false;
    geom.infinite = true;
    geom.wo = wo;
    return geom;
  }

  static PointGeometry make_on_surface(const Vector3 &p, const Vector3 &ng,
                                       const Vector3 &ns, const Vector2 &uv) {
    PointGeometry geom;
    geom.degenerated = false;
    geom.infinite = false;
    geom.p = p;
    geom.n = ng;
    geom.shading = Frame(ns);
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
  PointGeometry geom;            // Interaction point information
  const Shape *shape = nullptr;  // Scene interact with surface and records its shape information

  bool is_type(uint32_t type_flag) const {
    return (type & type_flag) > 0;
  }

  Vector3 to_world(const Vector3 &v) const { return geom.shading.to_world(v); }
  Vector3 to_local(const Vector3 &v) const { return geom.shading.to_local(v); }

  static SceneInteraction make_surface_interaction(const PointGeometry &geom, const Shape *shape) {
    SceneInteraction si;
    si.type = SurfaceInteraction;
    si.geom = geom;
    si.shape = shape;
    return si;
  }
};

}  // namespace misaki::render