#pragma once

#include "common/vector.hpp"

namespace common {

  struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray() : origin(), direction() { }

    Ray(Vec3 const & o, Vec3 const & d) : origin(o), direction(d) { }

    // punto del rayo a parámetro t
    Vec3 at(double t) const {
      return Vec3(origin.x + t * direction.x, origin.y + t * direction.y,
                  origin.z + t * direction.z);
    }
  };

}  // namespace common
