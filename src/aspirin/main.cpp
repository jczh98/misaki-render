#include <aspirin/xml.h>
#include <aspirin/object.h>
#include <aspirin/scene.h>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace aspirin;

int main(int argc, char** argv) {
    Class::static_initialization();
    library_nop();
    fs::path path = "../../../assets/bunny/scene.xml";
    get_file_resolver()->append(fs::path(argv[0]).parent_path());
    get_file_resolver()->append(path.parent_path());
    try {
        auto scene = xml::load_file(get_file_resolver()->resolve(path), "scalar_rgb");
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    Class::static_shutdown();
    return 0;
}