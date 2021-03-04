#pragma once

#include "fwd.h"

namespace aspirin::warp {

template <typename Value>
APR_INLINE Eigen::Matrix<Value, 2, 1> square_to_uniform_triangle(const Eigen::Matrix<Value, 2, 1> &sample) {
  Value t = math::safe_sqrt(1.f - sample.x());
  return Eigen::Matrix<Value, 2, 1>(1.f - t, t * sample.y());
}

template <typename Value>
APR_INLINE Eigen::Matrix<Value, 2, 1> square_to_uniform_disk_concentric(const Eigen::Matrix<Value, 2, 1> &sample) {
  Value x = 2.f * sample.x() - 1.f;
  Value y = 2.f * sample.y() - 1.f;
  Value phi, r;
  if (x == 0 && y == 0) {
    r = phi = 0;
  } else if (x * x > y * y) {
    r = x;
    phi = (math::Pi<Value> / 4.f) * (y / x);
  } else {
    r = y;
    phi = (math::Pi<Value> / 2.f) - (x / y) * (math::Pi<Value> / 4.f);
  }
  return Eigen::Matrix<Value, 2, 1>(r * std::cos(phi), r * std::sin(phi));
}

template <typename Value>
APR_INLINE Eigen::Matrix<Value, 3, 1> square_to_cosine_hemisphere(const Eigen::Matrix<Value, 2, 1> &sample) {
  auto p = square_to_uniform_disk_concentric<Value>(sample);
  Value z = math::safe_sqrt(1.f - p.squaredNorm());
  return Eigen::Matrix<Value, 3, 1>(p.x(), p.y(), z);
}

template <typename Value>
APR_INLINE Value square_to_cosine_hemisphere_pdf(const Eigen::Matrix<Value, 3, 1> &v) {
  return math::InvPi<Value> * v.z();
}
}