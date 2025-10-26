#include "render/ppm.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
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

  inline int to_byte_gamma2(double v01) {
    // gamma 2.0 -> levantar a 1/2
    double g = std::sqrt(clamp01(v01));
    int vi   = static_cast<int>(std::lround(g * 255.0));
    if (vi < 0) {
      vi = 0;
    }
    if (vi > 255) {
      vi = 255;
    }
    return vi;
  }

}  // namespace

namespace render {

  bool write_ppm_gamma(
      std::string const & path, int width, int height,
      std::function<void(int, int, double &, double &, double &)> const & sampler) {
    if (width <= 0 || height <= 0) {
      return false;
    }
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
      return false;
    }

    out << "P3\n" << width << " " << height << "\n255\n";

    double r = 0, g = 0, b = 0;
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        sampler(x, y, r, g, b);
        int R = to_byte_gamma2(r);
        int G = to_byte_gamma2(g);
        int B = to_byte_gamma2(b);
        out << R << ' ' << G << ' ' << B << '\n';
      }
    }

    return static_cast<bool>(out);
  }

}  // namespace render
