#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace render {

  struct ImageSOA {
    int width, height;
    std::vector<uint8_t> R, G, B;

    explicit ImageSOA(int w, int h) : width(w), height(h) {
      std::size_t const n = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
      R.assign(n, 0);
      G.assign(n, 0);
      B.assign(n, 0);
    }

    inline std::size_t index(int x, int y) const {
      return static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
             static_cast<std::size_t>(x);
    }

    void set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
      std::size_t const i = index(x, y);
      R[i]                = r;
      G[i]                = g;
      B[i]                = b;
    }

    void get(int x, int y, uint8_t & r, uint8_t & g, uint8_t & b) const {
      std::size_t const i = index(x, y);
      r                   = R[i];
      g                   = G[i];
      b                   = B[i];
    }

    static uint8_t clamp01_to_u8(double v) {
      v            = std::clamp(v, 0.0, 1.0);
      int const vi = static_cast<int>(std::lround(v * 255.0));
      return static_cast<uint8_t>(std::clamp(vi, 0, 255));
    }

    void set01(int x, int y, double r, double g, double b) {
      set(x, y, clamp01_to_u8(r), clamp01_to_u8(g), clamp01_to_u8(b));
    }
  };

}  // namespace render
