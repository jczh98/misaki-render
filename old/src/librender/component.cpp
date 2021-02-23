#include <misaki/render/component.h>

namespace misaki::render {

std::string Component::to_string() const {
  return refl::type::get(*this).get_name().to_string();
}

system::FileResolver *get_file_resolver() {
  static system::FileResolver file_resolver;
  return &file_resolver;
}

RTTR_REGISTRATION {
  refl::registration::class_<Component>("Component")
      .constructor<>();
}

}  // namespace misaki::render