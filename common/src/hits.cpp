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

    // ==== TAPA INFERIOR (plano en la base) ====
    // Usa solo nombres que existen en tu firma: base, axis, radius, t_min, t_max, *t_hit, *n_hit.
    {
      // Ojo: adapta estos dos si en tu ray son r.orig / r.dir en vez de r.origin / r.direction
      render::vector const & o = r.origin;
      render::vector const & d = r.direction;

      render::vector const p0 = base;  // centro de la tapa inferior
      render::vector const n  = axis;  // normal de la tapa (asumo axis normalizado)

      double const denom = n.dot(d);  // si tu vector no tiene .dot, usa (n * d) si lo sobrecargaste
      if (std::abs(denom) > 1e-12) {
        double const t = (p0 - o).dot(n) / denom;
        if (t >= t_min and t <= t_max) {
          render::vector const p = o + d * t;

          // ¿p está dentro del disco de radio 'radius'?
          // Quita la componente paralela a 'axis' y mide la longitud de la perpendicular.
          render::vector const v      = p - p0;
          render::vector const v_perp = v - n * v.dot(n);
          double const r2             = v_perp.dot(v_perp);

          if (r2 <= radius * radius + 1e-12) {
            // Si es el más cercano, actualiza salida
            if (t < *t_out) {
              *t_out      = t;
              *normal_out = n;  // normal de la tapa inferior
              // Orienta normal contra el rayo (convención habitual)
              if (normal_out->dot(d) > 0.0) {
                *normal_out = (*normal_out) * (-1.0);
              }
            }
          }
        }
      }
    }

    // ==== TAPA SUPERIOR (plano en la parte alta) ====
    // p0_top = base + height * axis
    {
      render::vector const & o = r.origin;
      render::vector const & d = r.direction;

      render::vector const p0_top = base + axis * height;  // centro de la tapa superior
      render::vector const n_top  = axis * (-1.0);  // normal hacia fuera (opuesta a la inferior)

      double const denom = n_top.dot(d);
      if (std::abs(denom) > 1e-12) {
        double const t = (p0_top - o).dot(n_top) / denom;
        if (t >= t_min and t <= t_max) {
          render::vector const p = o + d * t;

          // Dentro del disco de radio 'radius'?
          render::vector const v      = p - p0_top;
          render::vector const v_perp = v - axis * v.dot(axis);
          double const r2             = v_perp.dot(v_perp);

          if (r2 <= radius * radius + 1e-12) {
            if (t < *t_out) {
              *t_out      = t;
              *normal_out = n_top;
              // orienta la normal contra el rayo (convención)
              if (normal_out->dot(d) > 0.0) {
                *normal_out = (*normal_out) * (-1.0);
              }
            }
          }
        }
      }
    }

    return false;
  }

}  // namespace render
