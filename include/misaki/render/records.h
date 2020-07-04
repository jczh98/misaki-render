#pragma once

#include "interaction.h"
#include "ray.h"

namespace misaki::render {

struct PositionSample {
  PointGeometry geom;
};

struct RaySample {
  Ray ray;
  Color3 weight;
};

struct DirectSample {
  PointGeometry geom;
  Float pdf{0.f};
  Vector3 d{0.f};
  Float dist{0.f};
  static DirectSample make_with_interactions(const SceneInteraction &sampled, const SceneInteraction &ref) {
    DirectSample ds;
    ds.geom = sampled.geom;
    ds.d = sampled.geom.p - ref.geom.p;
    ds.dist = math::norm(ds.d);
    ds.d /= ds.dist;
    if (!sampled.is_valid()) ds.d = -sampled.wi;
    return ds;
  }
};

}  // namespace misaki::render