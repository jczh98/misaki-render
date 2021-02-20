#pragma once

#include "bbox.h"
#include "transform.h"
#include <Eigen/Dense>

namespace nekoba {

using Float = float;

using Vector2f = Eigen::Vector2f;
using Vector3f = Eigen::Vector3f;
using Vector4f = Eigen::Vector4f;

using Vector2 = Eigen::Matrix<Float, 2, 1>;
using Vector3 = Eigen::Matrix<Float, 3, 1>;
using Vector4 = Eigen::Matrix<Float, 4, 1>;

using Vector2i = Eigen::Vector2i;
using Vector3i = Eigen::Vector3i;
using Vector4i = Eigen::Vector4i;

using Vector2u = Eigen::Matrix<uint32_t, 2, 1>;
using Vector3u = Eigen::Matrix<uint32_t, 3, 1>;
using Vector4u = Eigen::Matrix<uint32_t, 4, 1>;

using Matrix3 = Eigen::Matrix<Float, 3, 3>;
using Matrix4 = Eigen::Matrix<Float, 4, 4>;
using Matrix3f = Eigen::Matrix<float, 3, 3>;
using Matrix4f = Eigen::Matrix<float, 4, 4>;

using Transform3 = Transform<Float, 3>;
using Transform4 = Transform<Float, 4>;

using BoundingBox3 = BoundingBox<Float, 3>;
using BoundingSphere3 = BoundingSphere<Float, 3>;

template <typename Value, size_t Size> struct Color;

template <typename Spectrum> class BSDF;
template <typename Spectrum> class Emitter;
template <typename Spectrum> class Endpoint;
template <typename Spectrum> class Film;
template <typename Spectrum> class ImageBlock;
template <typename Spectrum> class Integrator;
template <typename Spectrum> class Mesh;
template <typename Spectrum> class Shape;
template <typename Spectrum> class ReconstructionFilter;
template <typename Spectrum> class Sampler;
template <typename Spectrum> class Scene;
template <typename Spectrum> class Sensor;
template <typename Spectrum> class Texture;

} // namespace nekoba