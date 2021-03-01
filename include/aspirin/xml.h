#pragma once

#include "aspirin.h"

namespace aspirin::xml {

using ParameterList = std::vector<std::pair<std::string, std::string>>;

extern APR_EXPORT ref<Object> load_file(const fs::path &path,
                                        const std::string &variant,
                                        ParameterList parameters = {});

} // namespace aspirin::xml