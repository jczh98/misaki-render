#pragma once

#include "fwd.h"
#include "component.h"

namespace misaki::render {

class MSK_EXPORT PluginManager {
 public:
  static PluginManager *instance() {
    static PluginManager instance;
    return &instance;
  }
  PluginManager();
  ~PluginManager();
  std::shared_ptr<Component> create_comp(const Properties &);
  std::shared_ptr<Component> create_comp(const Properties &, const refl::type &);
  template <typename T, std::enable_if_t<!std::is_same_v<T, Component>, int> = 0>
  std::shared_ptr<T> create_comp(const Properties &props) {
    auto type = refl::type::get<T>();
    return std::dynamic_pointer_cast<T>(create_comp(props, type));
  }
 private:
  struct PluginManagerPrivate;
  std::unique_ptr<PluginManagerPrivate> d;
};

}