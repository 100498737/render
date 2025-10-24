#pragma once

#include <cmath>
#include <cstdint>
#include <ostream>

namespace common {

  struct Vec3 {
    double x;
    double y;
    double z;

    // --- Constructores ---
    Vec3() : x(0.0), y(0.0), z(0.0) { }

    Vec3(double x_value, double y_value, double z_value) : x(x_value), y(y_value), z(z_value) { }

    // --- Operador unario (-v): invierte el signo del vector ---
    [[nodiscard]] Vec3 operator-() const { return Vec3(-x, -y, -z); }

    // --- Suma y resta vectoriales ---
    [[nodiscard]] Vec3 operator+(Vec3 const & other) const {
      return Vec3(x + other.x, y + other.y, z + other.z);
    }

    [[nodiscard]] Vec3 operator-(Vec3 const & other) const {
      return Vec3(x - other.x, y - other.y, z - other.z);
    }

    // --- Producto y división por escalar ---
    [[nodiscard]] Vec3 operator*(double scalar) const {
      return Vec3(x * scalar, y * scalar, z * scalar);
    }

    [[nodiscard]] Vec3 operator/(double scalar) const {
      double inv = 1.0 / scalar;
      return Vec3(x * inv, y * inv, z * inv);
    }

    // --- Producto escalar (dot) ---
    [[nodiscard]] double dot(Vec3 const & other) const {
      return (x * other.x) + (y * other.y) + (z * other.z);
    }

    // --- Producto vectorial (cross) ---
    [[nodiscard]] Vec3 cross(Vec3 const & other) const {
      return Vec3((y * other.z) - (z * other.y), (z * other.x) - (x * other.z),
                  (x * other.y) - (y * other.x));
    }

    // --- Magnitud / norma euclídea ---
    [[nodiscard]] double magnitude() const { return std::sqrt(this->dot(*this)); }

    // --- Versión normalizada del vector ---
    [[nodiscard]] Vec3 normalized() const {
      double m = magnitude();
      if (m == 0.0) {
        return *this;  // evita dividir por cero
      }
      return *this / m;
    }
  };

  // --- Escalar * vector (simetría con v * escalar) ---
  inline Vec3 operator*(double scalar, Vec3 const & v) {
    return v * scalar;
  }

  // --- Sobrecarga del operador << para imprimir ---
  inline std::ostream & operator<<(std::ostream & os, Vec3 const & v) {
    os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
    return os;
  }

}  // namespace common
