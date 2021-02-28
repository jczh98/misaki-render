#include <aspirin/xml.h>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace aspirin;

int main() {
    try {
        auto scene = xml::load_file("../../../assets/bunny/scene.xml");
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}