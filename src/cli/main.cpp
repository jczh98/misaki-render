#include <iostream>

#include <misaki/render/component.h>
#include <misaki/render/logger.h>
#include <misaki/render/properties.h>
#include <misaki/render/camera.h>
#include <misaki/render/plugin.h>
#include <rttr/registration.h>

using namespace misaki::render;

int main() {
  Properties props("perspective");
  try {
    auto perspective = PluginManager::instance()->create_comp<Camera>(props);
    Log(Info, perspective->to_string());
  } catch (std::exception &e) {
    Log(Info, e.what());
  }
  misaki::math::TVector<float, 3> v = {1, 2, 3};
  Log(Info, v.to_string());
  return 0;
}
