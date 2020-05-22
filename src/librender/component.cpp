#include <misaki/render/component.h>

namespace misaki::render {

std::string Component::to_string() const {
  return refl::type::get(*this).get_name().to_string();
}

}