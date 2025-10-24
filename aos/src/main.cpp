#include <print>

#include "vector.hpp"

int main(int argc, char **) {
  // Requisito: exactamente 3 argumentos (cfg, scene, out)
  if (argc != 4) {
    std::println(stderr, "Error: Invalid number of arguments: {}", argc - 1);
    return 1;
  }
  std::println("Starting AOS rendering");
  render::vector vec{1.0, 2.0, 3.0};
  std::println("Vector magnitude: {}", vec.magnitude());
  return 0;
}
