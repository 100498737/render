#pragma once
#include <cmath>

namespace render {

  // --- Constantes numéricas globales (alineadas con el enunciado) ---
  // EPS_HIT  -> evita autointersecciones de rayos recién generados
  // EPS_TINY -> evita divisiones por ~0 al normalizar vectores
  constexpr double EPS_HIT  = 1e-3;
  constexpr double EPS_TINY = 1e-8;

  // --- Clase base: vector 3D ---
  struct vector {
    double x{};
    double y{};
    double z{};

    // Constructores
    constexpr vector() = default;

    constexpr vector(double x_, double y_, double z_) : x(x_), y(y_), z(z_) { }

    // --- Operadores básicos ---
    [[nodiscard]] constexpr vector operator+(vector const & other) const {
      return vector{x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr vector operator-(vector const & other) const {
      return vector{x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr vector operator*(double s) const { return vector{x * s, y * s, z * s}; }

    [[nodiscard]] constexpr vector operator/(double s) const { return vector{x / s, y / s, z / s}; }

    // --- Producto escalar y vectorial ---
    [[nodiscard]] constexpr double dot(vector const & other) const {
      return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr vector cross(vector const & other) const {
      return vector{y * other.z - z * other.y, z * other.x - x * other.z,
                    x * other.y - y * other.x};
    }

    // --- Magnitud (longitud euclídea) ---
    [[nodiscard]] double magnitude() const { return std::sqrt(dot(*this)); }

    // --- Normalización (versor) ---
    [[nodiscard]] vector normalized() const {
      double const len = magnitude();
      if (len < EPS_TINY) {
        // Evitar división por casi 0 → devolvemos el propio vector
        return *this;
      }
      return *this / len;
    }
  };

  // --- Operador escalar * vector (por la izquierda) ---
  [[nodiscard]] inline constexpr vector operator*(double s, vector const & v) {
    return vector{v.x * s, v.y * s, v.z * s};
  }

  // --- Alias oficial del enunciado ---
  using Vec3 = vector;

}  // namespace render
