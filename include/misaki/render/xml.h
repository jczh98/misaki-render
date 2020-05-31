#pragma once

#include "fwd.h"

namespace misaki::render::xml {

using ParameterList = std::vector<std::pair<std::string, std::string>>;

extern MSK_EXPORT std::shared_ptr<Component> load_file(const fs::path &path, ParameterList parameters = {});

}