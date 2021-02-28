#pragma once

#include "fwd.h"
#include "component.h"

namespace aspirin::xml {

using ParameterList = std::vector<std::pair<std::string, std::string>>;

extern APR_EXPORT std::shared_ptr<Component> load_file(const fs::path &path, ParameterList parameters = {});

}