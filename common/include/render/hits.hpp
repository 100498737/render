#pragma once
#include "render/ray.hpp"
#include "render/vector.hpp"

namespace render {

  bool hit_sphere(ray const & r, vector const & center, double radius, double t_min, double t_max,
                  double * t_out, vector * normal_out);

}  // namespace render
