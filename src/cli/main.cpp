#include <misaki/render/camera.h>
#include <misaki/render/component.h>
#include <misaki/render/logger.h>
#include <misaki/render/plugin.h>
#include <misaki/render/properties.h>

#include <iostream>

using namespace misaki::render;

int main(int argc, char** argv) {
  // Append executable directory to file resolver
  get_file_resolver()->append(fs::path(argv[0]).parent_path());
  Properties props("perspective");
  try {
    auto perspective = PluginManager::instance()->create_comp<Camera>(props);
    Log(Info, perspective->to_string());
  } catch (std::exception& e) {
    Log(Info, e.what());
  }
  misaki::math::TVector<float, 3> v = {1, 2, 3};
  misaki::math::Transform<float, 4> transform;
  auto vec = transform.apply_normal(v);
  auto rotation = misaki::math::Transform<float, 4>::scale({2, 2, 2});
  auto frame = misaki::math::Frame(v);
  Log(Info, rotation.to_string());
  Log(Info, v.to_string());
  Log(Info, frame.to_string());
  return 0;
}
