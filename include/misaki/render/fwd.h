#pragma once

#include <mutex>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <map>
#include <misaki/utils/math/vector.h>
#include <fmt/format.h>
#include <rttr/registration.h>
#include "platform.h"

namespace misaki::render {

namespace fs = std::filesystem;
namespace refl = rttr;

class Properties;
class Camera;

}