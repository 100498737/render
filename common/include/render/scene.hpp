#pragma once

#include <vector>

#include "render/vector.hpp"

namespace render {

  struct Sphere {
    Vec3 center{};
    double radius{};
  };

  struct Scene {
    std::vector<Sphere> spheres;
  };

}  // namespace render
