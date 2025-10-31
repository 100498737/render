#pragma once
#include "render/ray.hpp"
#include "render/vector.hpp"

namespace render {

  // rayo–esfera
  bool hit_sphere(ray const & r, vector const & center, double radius, double t_min, double t_max,
                  double * t_out, vector * normal_out);

  // rayo–cilindro  (OJO: height va ANTES que radius)
  bool hit_cylinder(ray const & r, vector const & base, vector const & axis, double height,
                    double radius, double t_min, double t_max, double * t_out, vector * normal_out);

}  // namespace render
