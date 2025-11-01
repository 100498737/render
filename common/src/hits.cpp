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
    // helpers locales
    auto length    = [](vector const & v) -> double { return std::sqrt(v.dot(v)); };
    auto normalize = [&](vector v) -> vector {
      double L = length(v);
      return (L > 0.0) ? (1.0 / L) * v : v;
    };

    // Normaliza el eje (por si nos llega sin normalizar)
    vector ax = normalize(axis);
    vector O  = r.origin;
    vector D  = r.direction;         // se asume ya normalizada en tu cámara
    vector P0 = base;                // punto de la tapa inferior
    vector P1 = base + height * ax;  // punto de la tapa superior

    bool hit_any     = false;
    double best_t    = t_max;
    vector best_norm = {0, 0, 0};

    // --- 1) Intersección con el lateral ---------------------------------------
    // Proyecta origen y dirección al plano perpendicular a ax
    double D_par = D.dot(ax);
    double O_par = (O - P0).dot(ax);

    vector D_perp = D - D_par * ax;  // componente perpendicular del rayo
    vector O_perp = (O - P0) - O_par * ax;

    double a = D_perp.dot(D_perp);
    double b = 2.0 * O_perp.dot(D_perp);
    double c = O_perp.dot(O_perp) - radius * radius;

    if (a > 1e-16) {
      double disc = b * b - 4.0 * a * c;
      if (disc >= 0.0) {
        double sdisc = std::sqrt(disc);
        // Dos candidatos
        double t0 = (-b - sdisc) / (2.0 * a);
        double t1 = (-b + sdisc) / (2.0 * a);

        auto try_t_lateral = [&](double t) {
          if (t < t_min || t > best_t) {
            return;
          }
          // Comprueba que el punto cae entre tapas: 0 <= y <= height
          double y = O_par + t * D_par;  // coordenada a lo largo de ax relativa a P0
          if (y < 0.0 || y > height) {
            return;
          }

          // Punto de impacto y normal lateral (componente perpendicular normalizada)
          vector P  = O + t * D;
          vector Pc = P - (P0 + y * ax);  // vector radial en el plano perpendicular
          vector N  = normalize(Pc);      // salida hacia fuera

          best_t    = t;
          best_norm = N;
          hit_any   = true;
        };

        // pruebo ambos
        if (t0 > t_min) {
          try_t_lateral(t0);
        }
        if (t1 > t_min) {
          try_t_lateral(t1);
        }
      }
    }

    // --- 2) Intersección con TAPA inferior (plano por P0, normal -ax hacia fuera) ---
    {
      double denom = ax.dot(D);       // D·ax
      if (std::abs(denom) > 1e-16) {  // no paralelo
        double t = (P0 - O).dot(ax) / denom;
        if (t >= t_min && t < best_t) {
          vector P = O + t * D;
          // distancia radial al centro de la tapa inferior
          vector radial = P - P0 - ((P - P0).dot(ax)) * ax;  // componente perpendicular
          if (radial.dot(radial) <= radius * radius + 1e-12) {
            best_t    = t;
            best_norm = -1.0 * ax;  // normal hacia fuera en la tapa inferior
            hit_any   = true;
          }
        }
      }
    }

    // --- 3) Intersección con TAPA superior (plano por P1, normal +ax hacia fuera) ---
    {
      double denom = ax.dot(D);
      if (std::abs(denom) > 1e-16) {
        double t = (P1 - O).dot(ax) / denom;
        if (t >= t_min && t < best_t) {
          vector P      = O + t * D;
          vector radial = P - P1 - ((P - P1).dot(ax)) * ax;
          if (radial.dot(radial) <= radius * radius + 1e-12) {
            best_t    = t;
            best_norm = ax;  // normal hacia fuera en la tapa superior
            hit_any   = true;
          }
        }
      }
    }

    if (!hit_any) {
      return false;
    }
    if (t_out) {
      *t_out = best_t;
    }
    if (normal_out) {
      *normal_out = best_norm;
    }
    return true;
  }

}  // namespace render
