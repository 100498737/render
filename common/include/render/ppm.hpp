#pragma once
#include <functional>
#include <string>

namespace render {

  // ── Versión ORIGINAL (4 parámetros) que usan los tests ──────────────────────
  bool write_ppm_gamma(std::string const & path, int width, int height,
                       std::function<void(int, int, double &, double &, double &)> const & sampler);

  // ── NUEVA sobrecarga (5 parámetros) con gamma configurable ──────────────────
  bool write_ppm_gamma(std::string const & path, int width, int height, double gamma,
                       std::function<void(int, int, double &, double &, double &)> const & sampler);

}  // namespace render
