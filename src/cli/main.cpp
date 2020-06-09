#include <misaki/render/camera.h>
#include <misaki/render/component.h>
#include <misaki/render/logger.h>
#include <misaki/render/plugin.h>
#include <misaki/render/properties.h>
#include <misaki/render/xml.h>
#include <misaki/render/scene.h>
#include <iostream>

using namespace misaki::render;

int main(int argc, char** argv) {
  // Append executable directory to file resolver
  get_file_resolver()->append(fs::path(argv[0]).parent_path());
  try {
    auto comp = xml::load_file(get_file_resolver()->resolve("assets/scene.xml"));
    auto scene = std::dynamic_pointer_cast<Scene>(comp);
    Log(Info, "load success");
  } catch (std::exception& e) {
    Log(Info, e.what());
  }
  misaki::math::TVector<float, 3> v = {1, 2, 3};
  misaki::math::Transform<float, 4> transform;
  auto vec = transform.apply_normal(v);
  auto rotation = misaki::math::Transform<float, 4>::scale({2, 2, 2});
  auto frame = misaki::math::Frame(v);
  Color3 col = {1.5, 2., 3.};
  Properties test_prop;
  test_prop.set_vector3("vec3", v);
  test_prop.set_color("col", col);
  Log(Info, rotation.to_string());
  Log(Info, v.to_string());
  Log(Info, frame.to_string());
  Log(Info, col.to_string());
  Log(Info, test_prop.to_string());
  return 0;
}
