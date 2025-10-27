#pragma once
#include <string>
#include <vector>

#include "render/vector.hpp"

namespace render {

  enum class MaterialKind { Matte, Metal, Refractive };

  struct Material {
    std::string name;
    MaterialKind kind{MaterialKind::Matte};
    Vec3 color{1.0, 1.0, 1.0};  // matte/metal
    double fuzz{0.0};           // metal [0,1]
    double ior{1.5};            // refractive (>1)
  };

  struct Sphere {
    std::string name;
    Vec3 center{};
    double radius{0.0};
    std::string mat;  // nombre del material (vacío en legacy)
  };

  struct Cylinder {
    std::string name;
    Vec3 base{};
    Vec3 axis{};
    double height{0.0};
    double radius{0.0};
    std::string mat;
  };

  struct Scene {
    std::vector<Material> materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;
  };

}  // namespace render
