#pragma once

#include "fwd.h"

namespace aspirin::xml {

using ParameterList = std::vector<std::pair<std::string, std::string>>;

extern APR_EXPORT ref<Object> load_file(const fs::path &path,
                                        ParameterList parameters = {});

} // namespace aspirin::xml