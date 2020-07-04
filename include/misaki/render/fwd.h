#pragma once

#include <fmt/format.h>
#include <misaki/utils/math.h>
#include <misaki/utils/system.h>
#include <misaki/utils/texture.h>
#include <misaki/utils/util.h>
#include <rttr/registration.h>
#include <rttr/rttr_enable.h>

#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <variant>
#include <vector>

#include "platform.h"

namespace misaki::render {

namespace fs = std::filesystem;
namespace refl = rttr;

class Component;
class Properties;
class Camera;
class Light;
class Integrator;
class SceneInteraction;
class Film;
class ReconstructionFilter;
class Sampler;
class BSDF;
class Scene;
class Shape;
class Mesh;
struct Ray;
struct PointGeometry;
struct DirectSample;
struct RaySample;

using Float = float;

template <typename T>
constexpr auto Infinity = std::numeric_limits<T>::infinity();
template <typename T>
constexpr auto Epsilon = std::numeric_limits<T>::epsilon() / 2;
template <typename T>
constexpr auto RayEpsilon = Epsilon<T> * 1500;
template <typename T>
constexpr auto ShadowEpsilon = RayEpsilon<T> * 10;
template <typename T>
constexpr auto Pi = T(3.14159265358979323846);
template <typename T>
constexpr auto InvPi = T(0.31830988618379067154);
template <typename T>
constexpr auto InvTwoPi = T(0.15915494309189533577);
template <typename T>
constexpr auto InvFourPi = T(0.07957747154594766788);

using Vector2f = math::TVector<float, 2>;
using Vector3f = math::TVector<float, 3>;
using Vector4f = math::TVector<float, 4>;

using Vector2 = math::TVector<Float, 2>;
using Vector3 = math::TVector<Float, 3>;
using Vector4 = math::TVector<Float, 4>;

using Vector2i = math::TVector<int, 2>;
using Vector3i = math::TVector<int, 3>;
using Vector4i = math::TVector<int, 4>;

using Vector2u = math::TVector<uint64_t, 2>;
using Vector3u = math::TVector<uint64_t, 3>;
using Vector4u = math::TVector<uint64_t, 4>;

using Matrix3 = math::TMatrix<Float, 3>;
using Matrix4 = math::TMatrix<Float, 4>;
using Matrix3f = math::TMatrix<float, 3>;
using Matrix4f = math::TMatrix<float, 4>;

using Transform3 = math::Transform<Float, 3>;
using Transform4 = math::Transform<Float, 4>;
using Transform3f = math::Transform<float, 3>;
using Transform4f = math::Transform<float, 4>;

using Color3 = math::TColor<Float, 3>;
using Color4 = math::TColor<Float, 4>;

using BoundingBox3 = math::TBoundingBox<Float, 3>;
using BoundingSphere3 = math::TBoundingSphere<Float, 3>;

using Frame = math::TFrame<Float>;

using Distribution1D = math::Distribution1D<Float>;
using Distribution2D = math::Distribution2D<Float>;

#define MSK_ENABLE_EMBREE 1
}  // namespace misaki::render