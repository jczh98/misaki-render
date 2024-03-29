#include <iostream>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/object.h>
#include <misaki/core/xml.h>
#include <misaki/core/image.h>
#include <misaki/render/integrator.h>
#include <misaki/render/scene.h>
#include <misaki/render/sensor.h>
#include <misaki/ui/viewer.h>
#include <spdlog/spdlog.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/global_control.h>

using namespace misaki;

const bool gui = false;

bool render(Object *scene_, fs::path filename) {
    auto *scene = dynamic_cast<Scene *>(scene_);
    if (!scene) {
        Throw("Root element of the input file must be a <scene> tag!");
    }
    auto sensor = scene->sensor();
    auto film   = sensor->film();
    film->set_destination_file(filename);
    auto integrator = scene->integrator();
    if (!integrator)
        Throw("No integrator specified for scene");

    ui::Viewer viewer(film);

    if (gui) {
        viewer.init();
    }

    std::thread render_thread([&] {
        bool success = integrator->render(scene, sensor);
        if (success) {
            film->develop();
        } else {
            Log(Warn, "Rendering failed, result not saved.");
        }
    });

    if (gui) {
        viewer.mainloop();
    }

    render_thread.join();

    if (gui) {
        viewer.shutdown();
    }

    return true;
}

int main(int argc, char **argv) {
    tbb::global_control global_limit(
        tbb::global_control::max_allowed_parallelism, 8);
    Class::static_initialization();
    InstanceManager::static_initialization();
    library_nop();

    fs::path path = "../../../results/Figure_1_Pathtrace/scene.xml";
    get_file_resolver()->append(fs::path(argv[0]).parent_path());
    get_file_resolver()->append(path.parent_path());
    try {
        auto scene   = xml::load_file(get_file_resolver()->resolve(path));
        bool success = render(scene.get(), path);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    InstanceManager::static_shutdown();
    Class::static_shutdown();
    return 0;
}