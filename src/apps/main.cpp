#include <misaki/integrator.h>
#include <misaki/logger.h>
#include <misaki/object.h>
#include <misaki/scene.h>
#include <misaki/sensor.h>
#include <misaki/xml.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <tbb/task_scheduler_init.h>

using namespace misaki;

bool render(Object *scene_, fs::path filename) {
    auto *scene = dynamic_cast<Scene *>(scene_);
    if (!scene) {
        Throw("Root element of the input file must be a <scene> tag!");
    }
    auto sensor = scene->sensor();
    auto film   = sensor->film();
    filename.replace_extension("png");
    film->set_destination_file(filename);
    auto integrator = scene->integrator();
    if (!integrator)
        Throw("No integrator specified for scene");
    bool success = integrator->render(scene, sensor);
    if (success) {
        film->develop();
    } else {
        Log(Warn, "Rendering failed, result not saved.");
    }
    return success;
}

int main(int argc, char **argv) {
    Class::static_initialization();
    library_nop();
    fs::path path = "../../assets/cbox/scene.xml";
    get_file_resolver()->append(fs::path(argv[0]).parent_path());
    get_file_resolver()->append(path.parent_path());
    try {
        auto scene   = xml::load_file(get_file_resolver()->resolve(path));
        bool success = render(scene.get(), path);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    Class::static_shutdown();
    return 0;
}