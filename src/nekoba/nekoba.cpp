#include <iostream>
#include <nekoba/render/logger.h>
#include <spdlog/spdlog.h>

using namespace nekoba;

int main() {
  Transform4 transform = Transform4::translate(Vector3(1, 1, 1));
  std::cout << transform << std::endl;
  Log(Info, "nekoba");
  return 0;
}