#include "render/hits.hpp"
#include <cmath>

namespace render {

  bool hit_sphere(ray const & r, vector const & center, double radius, double t_min, double t_max,
                  double * t_out, vector * normal_out) {
    // Ecuación: ||(o + t d) - c||^2 = r^2
    // Sea oc = o - c. a = d·d, b = 2 oc·d, c2 = oc·oc - r^2
    vector const oc     = r.origin - center;
    double const a      = r.direction.dot(r.direction);
    double const half_b = oc.dot(r.direction);  // usamos forma con half_b para estabilidad
    double const c2     = oc.dot(oc) - radius * radius;

    double const discriminant = half_b * half_b - a * c2;
    if (discriminant < 0.0) {
      return false;
    }
    double const sqrt_disc = std::sqrt(discriminant);

    // primera raíz (la menor)
    double t = (-half_b - sqrt_disc) / a;
    if (t < t_min or t > t_max) {
      // prueba la segunda raíz
      t = (-half_b + sqrt_disc) / a;
      if (t < t_min or t > t_max) {
        return false;
      }
    }

    if (t_out != nullptr) {
      *t_out = t;
    }
    if (normal_out != nullptr) {
      vector const p = r.at(t);
      vector n       = (p - center) * (1.0 / radius);  // normalizada
      *normal_out    = n;
    }
    return true;
  }

  bool hit_cylinder(ray const & r, vector const & base, vector const & axis, double height,
                    double radius, double t_min, double t_max, double * t_out,
                    vector * normal_out) {
    // Normaliza el eje del cilindro
    vector k    = axis;
    double klen = std::sqrt(k.dot(k));
    if (klen == 0.0 or height <= 0.0 or radius <= 0.0) {
      return false;
    }
    k = k * (1.0 / klen);  // k = eje unitario

    // Descompón en componentes perpendiculares al eje
    vector d  = r.direction;
    vector oc = r.origin - base;

    double d_parallel = d.dot(k);
    vector d_perp     = d - k * d_parallel;

    double oc_parallel = oc.dot(k);
    vector oc_perp     = oc - k * oc_parallel;

    // Intersección con cilindro lateral: |oc_perp + t*d_perp|^2 = radius^2
    double A = d_perp.dot(d_perp);
    double B = 2.0 * oc_perp.dot(d_perp);
    double C = oc_perp.dot(oc_perp) - radius * radius;

    // Ray paralelo al eje (A ~ 0) => no corta la superficie lateral
    if (A <= 0.0) {
      return false;
    }

    double disc = B * B - 4.0 * A * C;
    if (disc < 0.0) {
      return false;
    }
    double sqrt_disc = std::sqrt(disc);

    auto within_axial = [&](double t) -> bool {
      if (t < t_min or t > t_max) {
        return false;
      }
      // Coordenada axial z (distancia sobre el eje desde 'base')
      double z = oc_parallel + t * d_parallel;
      // Excluimos bordes exactos (0 y height) para que no “toque” en el canto
      return (z > 0.0) and (z < height);
    };

    auto compute_normal = [&](double t, vector * n_out) {
      double z    = oc_parallel + t * d_parallel;
      vector p    = r.at(t);
      vector q    = base + k * z;  // proyección de p sobre el eje
      vector n    = p - q;         // radial
      double nlen = std::sqrt(n.dot(n));
      if (nlen > 0.0) {
        n = n * (1.0 / nlen);
      }
      if (n_out != nullptr) {
        *n_out = n;
      }
    };

    // Probar raíz cercana
    double t_candidate = (-B - sqrt_disc) / (2.0 * A);
    if (within_axial(t_candidate)) {
      if (t_out != nullptr) {
        *t_out = t_candidate;
      }
      compute_normal(t_candidate, normal_out);
      return true;
    }

    // Probar raíz lejana
    t_candidate = (-B + sqrt_disc) / (2.0 * A);
    if (within_axial(t_candidate)) {
      if (t_out != nullptr) {
        *t_out = t_candidate;
      }
      compute_normal(t_candidate, normal_out);
      return true;
    }

    return false;
  }

}  // namespace render
