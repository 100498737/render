#pragma once

#include "render/vector.hpp"
#include <vector>

namespace render {

  // Un objeto geométrico sencillo para TA2
  struct Sphere {
    vector center;
    double radius;
  };

  // Escena mínima: una lista de esferas
  struct Scene {
    std::vector<Sphere> spheres;
  };

}  // namespace render
