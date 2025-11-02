#pragma once
#include <string>
#include <vector>

#include "render/vector.hpp"

namespace render {

  enum class MaterialKind { Matte, Metal, Refractive };

  struct Material {
    std::string name;
    MaterialKind kind{MaterialKind::Matte};
    Vec3 color{1.0, 1.0, 1.0};  // usado en matte/metal
    double fuzz{0.0};           // solo metal [0,1]
    double ior{1.5};            // solo refractive (>1)
  };

  struct Sphere {
    std::string name;
    Vec3 center;
    double radius{0.0};
    std::string mat;  // nombre del material (puede estar vacío en legacy)
  };

  struct Cylinder {
    std::string name;
    Vec3 base;  // punto base
    Vec3 axis;  // vector eje (se normaliza)
    double height{0.0};
    double radius{0.0};
    std::string mat;  // nombre del material
  };

  struct Scene {
    std::vector<Material> materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;
  };

  struct SceneStats {
    std::size_t spheres, cylinders;
  };

  SceneStats scene_stats(Scene const & scn);  // implementada en scene.cpp

}  // namespace render
