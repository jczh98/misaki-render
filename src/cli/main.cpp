#include <OpenImageIO/imageio.h>
#include <misaki/render/camera.h>
#include <misaki/render/component.h>
#include <misaki/render/integrator.h>
#include <misaki/render/logger.h>
#include <misaki/render/plugin.h>
#include <misaki/render/properties.h>
#include <misaki/render/scene.h>
#include <misaki/render/xml.h>
#include <misaki/render/bsdf.h>
#include <misaki/render/texture.h>
#include <tbb/task_scheduler_init.h>

#include <iostream>

using namespace misaki::render;

int main(int argc, char** argv) {
  //tbb::task_scheduler_init init(1);
  // Append executable directory to file resolver
  fs::path path = "assets/mitsuba/scene.xml";
  get_file_resolver()->append(fs::path(argv[0]).parent_path());
  get_file_resolver()->append(path.parent_path());
  try {
    auto comp = xml::load_file(get_file_resolver()->resolve(path));
    Log(Info, "load success");
    auto scene = std::dynamic_pointer_cast<Scene>(comp);
    auto film = scene->camera()->film();
    film->set_destination_file(path.replace_extension("png"));
    auto integrator = scene->integrator();
    integrator->render(scene);
    film->develop();
    Log(Info, "Finish rendering");
  } catch (std::exception& e) {
    std::cerr << e.what();
  }
  return 0;
}
