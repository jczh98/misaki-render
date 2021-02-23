#pragma once

#include "fwd.h"

namespace misaki::render {

namespace warp {

template <typename Value>
MSK_INLINE math::TVector<Value, 2> square_to_uniform_triangle(const math::TVector<Value, 2> &sample) {
  Value t = math::safe_sqrt(1.f - sample.x());
  return math::TVector<Value, 2>(1.f - t, t * sample.y());
}

template <typename Value>
MSK_INLINE math::TVector<Value, 2> square_to_uniform_disk_concentric(const math::TVector<Value, 2> &sample) {
  Value x = 2.f * sample.x() - 1.f;
  Value y = 2.f * sample.y() - 1.f;
  Value phi, r;
  if (x == 0 && y == 0) {
    r = phi = 0;
  } else if (x * x > y * y) {
    r = x;
    phi = (math::Pi<Float> / 4.f) * (y / x);
  } else {
    r = y;
    phi = (math::Pi<Float> / 2.f) - (x / y) * (math::Pi<Float> / 4.f);
  }
  return math::TVector<Value, 2>(r * std::cos(phi), r * std::sin(phi));
}

template <typename Value>
MSK_INLINE math::TVector<Value, 3> square_to_cosine_hemisphere(const math::TVector<Value, 2> &sample) {
  math::TVector<Value, 2> p = square_to_uniform_disk_concentric(sample);
  Value z = math::safe_sqrt(1.f - p.squaredNorm());
  return math::TVector<Value, 3>(p.x(), p.y(), z);
}

template <typename Value>
MSK_INLINE Value square_to_cosine_hemisphere_pdf(const math::TVector<Value, 3> &v) {
  return math::InvPi<Value> * v.z();
}
}

}