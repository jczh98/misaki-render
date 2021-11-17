#pragma once

#include "fwd.h"

namespace misaki::xml {

using ParameterList = std::vector<std::pair<std::string, std::string>>;

extern MSK_EXPORT ref<Object> load_file(const fs::path &path,
                                        ParameterList parameters = {});

} // namespace misaki::xml