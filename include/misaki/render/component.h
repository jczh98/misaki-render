#pragma once

#include "fwd.h"

namespace misaki::render {

#define MSK_DECL_COMP(...) RTTR_ENABLE(__VA_ARGS__)

class MSK_EXPORT Component {
 public:
  virtual std::string to_string() const;

  MSK_DECL_COMP()
};

extern MSK_EXPORT system::FileResolver *get_file_resolver();

#define MSK_REGISTER_CLASS(Name)                                  \
  RTTR_REGISTRATION {                                             \
    misaki::render::refl::registration::class_<Name>(#Name)       \
        .constructor<const Properties &>();                       \
  }

#define MSK_EXPORT_PLUGIN(Name)                                          \
  RTTR_PLUGIN_REGISTRATION {                                             \
    misaki::render::refl::registration::class_<Name>(#Name)              \
        .constructor<const Properties &>();                              \
  }                                                                      \
  extern "C" {                                                           \
  MSK_EXPORT const char *plugin_name() { return #Name; }                 \
  }

}  // namespace misaki::render
