#include <misaki/render/properties.h>

namespace misaki::render {

Properties::Properties() {

}
Properties::Properties(const std::string &plugin_name) : m_plugin_name(plugin_name) {
}
const std::string &Properties::plugin_name() const {
  return m_plugin_name;
}

}