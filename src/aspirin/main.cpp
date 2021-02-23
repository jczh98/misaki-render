#include <iostream>
#include <aspirin/logger.h>
#include <spdlog/spdlog.h>

using namespace aspirin;

int main() {
  Transform4 transform = Transform4::translate(Vector3(1, 1, 1));
  std::cout << transform << std::endl;
  Log(Info, "aspirin");
  return 0;
}