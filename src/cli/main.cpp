#include <misaki/render/camera.h>
#include <misaki/render/component.h>
#include <misaki/render/integrator.h>
#include <misaki/render/logger.h>
#include <misaki/render/plugin.h>
#include <misaki/render/properties.h>
#include <misaki/render/scene.h>
#include <misaki/render/xml.h>

#include <iostream>

using namespace misaki::render;

int main(int argc, char** argv) {
  // Append executable directory to file resolver
  get_file_resolver()->append(fs::path(argv[0]).parent_path());
  try {
    auto comp = xml::load_file(get_file_resolver()->resolve("assets/scene.xml"));
    auto scene = std::dynamic_pointer_cast<Scene>(comp);
    auto integrator = scene->integrator();
    integrator->render(scene);
    Log(Info, "load success");
  } catch (std::exception& e) {
    Log(Info, e.what());
  }
  return 0;
}
