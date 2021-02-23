#pragma once

#include "fwd.h"

namespace misaki::render {

struct Ray {
  Vector3 o, d;
  Float mint = RayEpsilon<Float>;
  Float maxt = Infinity<Float>;
  Float time = 0.f;
  Vector3 d_rcp;

  Ray() {}

  Ray(const Vector3 &o, const Vector3 &d, Float time) : o(o), d(d), time(time) { update(); }

  Ray(const Vector3 &o, const Vector3 &d, Float mint, Float maxt, Float time) : o(o), d(d), mint(mint), maxt(maxt), time(time) {
    update();
  }

  Vector3 operator()(Float t) const { return o + d * t; }

  void update() {
    d_rcp = 1.0 / d;
  }

  std::string to_string() const {
    std::ostringstream oss;
    oss << "Ray[" << std::endl;
    oss << "  o = " << string::indent(o.to_string(), 6) << "," << std::endl;
    oss << "  d = " << string::indent(d.to_string(), 6) << "," << std::endl;
    oss << "  mint = " << mint << "," << std::endl;
    oss << "  maxt = " << maxt << "," << std::endl;
    oss << "  time = " << time << std::endl;
    oss << "]";
    return oss.str();
  }
};

struct RayDifferential : Ray {
  Vector3 o_x, o_y, d_x, d_y;
  bool has_differentials = false;

  void scale_differential(Float amount) {
    o_x = (o_x - o) * amount + o;
    o_y = (o_y - o) * amount + o;
    d_x = (d_x - d) * amount + d;
    d_y = (d_y - d) * amount + d;
  }
  RayDifferential() {}
  RayDifferential(const Ray &ray) : Ray(ray), has_differentials(false) {}
};

}  // namespace misaki::render