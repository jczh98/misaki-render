#pragma once

#include <mutex>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <map>
#include <misaki/utils/math.h>
#include <misaki/utils/system.h>
#include <fmt/format.h>
#include <rttr/registration.h>
#include <rttr/rttr_enable.h>
#include "platform.h"

namespace misaki::render {

namespace fs = std::filesystem;
namespace refl = rttr;

class Properties;
class Camera;

using Float = float;
using Vector2f = math::TVector<float, 2>;
using Vector3f = math::TVector<float, 3>;
using Vector4f = math::TVector<float, 4>;

using Vector2 = math::TVector<Float, 2>;
using Vector3 = math::TVector<Float, 3>;
using Vector4 = math::TVector<Float, 4>;

using Matrix3f = math::TMatrix<float, 3>;
using Matrix4f = math::TMatrix<float, 4>;

}