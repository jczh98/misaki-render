#include <aspirin/xml.h>
#include <aspirin/object.h>
#include <aspirin/scene.h>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace aspirin;

int main() {
    Class::static_initialization();
    library_nop();
    try {
        auto scene = xml::load_file("../../../assets/bunny/scene.xml", "scalar_rgb");
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    Class::static_shutdown();
    return 0;
}