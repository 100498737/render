#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "common/vector.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char ** argv) {
  using common::Vec3;

  // Pequeña prueba para asegurar que el binario linka bien.
  Vec3 vec(1.0, 2.0, 3.0);

  std::cout << "vector magnitude: " << vec.magnitude() << "\n";

  return EXIT_SUCCESS;
}
