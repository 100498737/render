#pragma once
#include <cmath>
#include <iostream>
#include <print>

namespace render {

  // ====================================================================
  // Estructura base del vector 3D utilizada en todo el proyecto
  // ====================================================================
  struct vector {
    double x;
    double y;
    double z;

    // --- Constructores ---
    constexpr vector() : x(0.0), y(0.0), z(0.0) { }

    constexpr vector(double x_, double y_, double z_) : x(x_), y(y_), z(z_) { }

    // --- Operadores aritméticos básicos ---
    constexpr vector operator+(vector const & other) const {
      return {x + other.x, y + other.y, z + other.z};
    }

    constexpr vector operator-(vector const & other) const {
      return {x - other.x, y - other.y, z - other.z};
    }

    constexpr vector operator*(double scalar) const { return {x * scalar, y * scalar, z * scalar}; }

    constexpr vector operator/(double scalar) const { return {x / scalar, y / scalar, z / scalar}; }

    constexpr vector operator-() const { return {-x, -y, -z}; }

    // --- Operadores compuestos ---
    vector & operator+=(vector const & other) {
      x += other.x;
      y += other.y;
      z += other.z;
      return *this;
    }

    vector & operator-=(vector const & other) {
      x -= other.x;
      y -= other.y;
      z -= other.z;
      return *this;
    }

    vector & operator*=(double scalar) {
      x *= scalar;
      y *= scalar;
      z *= scalar;
      return *this;
    }

    vector & operator/=(double scalar) {
      x /= scalar;
      y /= scalar;
      z /= scalar;
      return *this;
    }

    // --- Producto punto y cruz ---
    [[nodiscard]] constexpr double dot(vector const & other) const {
      return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr vector cross(vector const & other) const {
      return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }

    // --- Magnitud y normalización ---
    [[nodiscard]] double magnitude() const { return std::sqrt(x * x + y * y + z * z); }

    [[nodiscard]] vector normalized() const {
      double mag = magnitude();
      return (mag > 1e-12) ? (*this / mag) : vector{};
    }

    // --- Utilidad para depuración ---
    void print() const { std::println("({}, {}, {})", x, y, z); }
  };

  // ====================================================================
  // Funciones auxiliares fuera del struct
  // ====================================================================

  [[nodiscard]] inline vector operator*(double scalar, vector const & v) {
    return {v.x * scalar, v.y * scalar, v.z * scalar};
  }

  // --- Alias de compatibilidad con otros módulos / compañeros ---
  using Vec3 = vector;  // 👈 Esto hace que render::Vec3 == render::vector

}  // namespace render
