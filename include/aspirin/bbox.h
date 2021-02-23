#pragma once

#include "bsphere.h"
#include <limits>

namespace aspirin {

template <typename Value, int Size> struct BoundingBox {
  using PointType = Eigen::Matrix<Value, Size, 1>;
  using VectorType = Eigen::Matrix<Value, Size, 1>;

  BoundingBox() { reset(); }
  BoundingBox(const PointType &p) {
    this->pmin = p;
    this->pmax = p;
  }
  BoundingBox(const PointType &pmin, const PointType &pmax) {
    this->pmin = pmin;
    this->pmax = pmax;
  }

  void reset() {
    pmin.setConstant(std::numeric_limits<Value>::infinity());
    pmax.setConstant(-std::numeric_limits<Value>::infinity());
  }

  template <typename T> void clip(const BoundingBox<T, Size> &bbox) {
    pmin = pmin.cwiseMax(bbox.pmin);
    pmax = pmax.cwiseMin(bbox.pmax);
  }

  template <typename T> void expand(const VectorType &p) {
    pmin = pmin.cwiseMin(p);
    pmax = pmax.cwiseMax(p);
  }

  template <typename T> void expand(const BoundingBox<T, Size> &bbox) {
    pmin = pmin.cwiseMin(bbox.pmin);
    pmax = pmax.cwiseMax(bbox.pmax);
  }

  PointType center() const { return (pmin + pmax) * Value(.5f); }

  BoundingSphere<Value, Size> bounding_sphere() const {
    PointType c = center();
    return {c, norm(c - pmax)};
  }

  PointType pmin, pmax;
};

} // namespace nekoba