#include <iostream>

#include <misaki/render/component.h>
#include <misaki/render/logger.h>

using namespace misaki::render;

int main() {
  librender_nop();
  misaki::math::TVector<float, 3> v = {1, 2, 3};
  Log(Info, v.to_string());
  return 0;
}
