#pragma once

#include <Eigen/Dense>

namespace nekoba {

template <typename Value_, int Size_> struct BoundingSphere {
  using Value = Value_;
  using PointType = Eigen::Matrix<Value_, Size_, 1>;
  PointType center;
  Value radius;
  BoundingSphere() : center(0.f), radius(0.f) {}
  BoundingSphere(const PointType &center, const Value &radius)
      : center(center), radius(radius) {}

  bool operator==(const BoundingSphere &bsphere) const {
    return center == bsphere.center && radius == bsphere.radius;
  }

  bool operator!=(const BoundingSphere &bsphere) const {
    return center != bsphere.center || radius != bsphere.radius;
  }

  bool empty() const { return radius <= 0.f; }
  void expand(const PointType &p) {
    radius = std::max<Value>(radius, norm(p - center));
  }
  bool contains(const PointType &p, bool strict = false) const {
    if (strict)
      return squared_norm(p - center) < radius * radius;
    else
      return squared_norm(p - center) <= radius * radius;
  }
};

} // namespace nekoba