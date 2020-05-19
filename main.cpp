#include <iostream>

#include <misaki/utils/math/vector.h>

int main() {
  misaki::math::TVector<float, 3> v = {1, 2, 3};
  std::cout << v << std::endl;
  return 0;
}
