#pragma once
#include "render/ray.hpp"
#include "render/vector.hpp"

namespace render {

  // Devuelve true si el rayo r golpea la esfera de centro 'center' y radio 'radius'
  // en el rango [t_min, t_max]. Si hay hit, escribe:
  //  - *t_out con el parámetro t
  //  - *normal_out con la normal unitaria en el punto de impacto (orientada hacia fuera).
  bool hit_sphere(ray const & r, vector const & center, double radius, double t_min, double t_max,
                  double * t_out, vector * normal_out);

  bool hit_cylinder(ray const & r, vector const & base, vector const & axis, double height,
                    double radius, double t_min, double t_max, double * t_out, vector * normal_out);

}  // namespace render
