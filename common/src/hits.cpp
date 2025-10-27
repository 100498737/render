#include "render/hits.hpp"
#include <cmath>

namespace render {

  bool hit_sphere(ray const & r, vector const & center, double radius, double t_min, double t_max,
                  double * t_out, vector * normal_out) {
    vector const oc     = r.origin - center;
    double const a      = r.direction.dot(r.direction);
    double const half_b = oc.dot(r.direction);
    double const c2     = oc.dot(oc) - radius * radius;

    double const disc = half_b * half_b - a * c2;
    if (disc < 0.0) {
      return false;
    }

    double const root = std::sqrt(disc);
    double t          = (-half_b - root) / a;
    if (t < t_min || t > t_max) {
      t = (-half_b + root) / a;
      if (t < t_min || t > t_max) {
        return false;
      }
    }

    if (t_out) {
      *t_out = t;
    }
    if (normal_out) {
      vector p    = r.at(t);
      *normal_out = (p - center) * (1.0 / radius);
    }
    return true;
  }

}  // namespace render
