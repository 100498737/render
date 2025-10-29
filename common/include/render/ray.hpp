#pragma once
#include "render/vector.hpp"

namespace render {

  struct ray {
    vector origin;
    vector direction;

    constexpr ray() = default;

    constexpr ray(vector const & o, vector const & d) : origin(o), direction(d) { }

    // Devuelve el punto P(t) = O + D * t
    [[nodiscard]] constexpr vector at(double t) const { return origin + direction * t; }
  };

}  // namespace render
