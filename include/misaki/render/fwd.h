#pragma once

#include <fmt/format.h>
#include <misaki/utils/math.h>
#include <misaki/utils/system.h>
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
class Sampler;
class BSDF;
class Scene;
class Shape;
class Mesh;

using Float = float;
using Vector2f = math::TVector<float, 2>;
using Vector3f = math::TVector<float, 3>;
using Vector4f = math::TVector<float, 4>;

using Vector2 = math::TVector<Float, 2>;
using Vector3 = math::TVector<Float, 3>;
using Vector4 = math::TVector<Float, 4>;

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

}  // namespace misaki::render