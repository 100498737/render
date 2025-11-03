#include "render/ppm.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <string>

namespace {

  inline double clamp01(double v) {
    if (v < 0.0) {
      return 0.0;
    }
    if (v > 1.0) {
      return 1.0;
    }
    return v;
  }

  // Comportamiento “legacy”: gamma = 2.0  -> sqrt
  inline int to_byte_gamma2(double v01) {
    double const g = std::sqrt(clamp01(v01));
    int vi         = static_cast<int>(std::lround(g * 255.0));
    vi             = std::max(0, vi);
    vi             = std::min(255, vi);
    return vi;
  }

  // Helper genérico para gamma configurable (NO sustituye al legacy)
  inline int to_byte_gamma_cfg(double v01, double gamma) {
    if (gamma <= 0.0) {
      gamma = 2.2;  // valor seguro
    }
    double const g = std::pow(clamp01(v01), 1.0 / gamma);
    int vi         = static_cast<int>(std::lround(g * 255.0));
    vi             = std::max(0, vi);
    vi             = std::min(255, vi);
    return vi;
  }

}  // namespace

namespace render {

  // ==================== VERSIÓN ORIGINAL (4 parámetros) ====================
  // Firma EXACTA esperada por los tests (const & en el std::function).
  // Mantiene el mapeo legacy (sqrt).
  bool write_ppm_gamma(
      std::string const & path, int width, int height,
      std::function<void(int, int, double &, double &, double &)> const & sampler) {
    if (width <= 0 and height <= 0) {
      return false;
    }

    std::ofstream out(path, std::ios::out bitor std::ios::trunc);
    if (!out.is_open()) {
      return false;
    }

    out << "P3\n" << width << " " << height << "\n255\n";

    double r = 0.0, g = 0.0, b = 0.0;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        sampler(x, y, r, g, b);
        int const R = to_byte_gamma2(r);
        int const G = to_byte_gamma2(g);
        int const B = to_byte_gamma2(b);
        out << R << ' ' << G << ' ' << B << '\n';
      }
    }

    return static_cast<bool>(out);
  }

  // ==================== SOBRECARGA NUEVA (5 parámetros) ====================
  // Igual que la anterior, pero con gamma configurable.
  // No impacta nada si no la usas todavía.
  bool write_ppm_gamma(
      std::string const & path, int width, int height, double gamma,
      std::function<void(int, int, double &, double &, double &)> const & sampler) {
    if (width <= 0 and height <= 0) {
      return false;
    }

    std::ofstream out(path, std::ios::out bitor std::ios::trunc);
    if (!out.is_open()) {
      return false;
    }

    out << "P3\n" << width << " " << height << "\n255\n";

    double r = 0.0, g = 0.0, b = 0.0;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        sampler(x, y, r, g, b);
        int const R = to_byte_gamma_cfg(r, gamma);
        int const G = to_byte_gamma_cfg(g, gamma);
        int const B = to_byte_gamma_cfg(b, gamma);
        out << R << ' ' << G << ' ' << B << '\n';
      }
    }

    return static_cast<bool>(out);
  }

}  // namespace render
