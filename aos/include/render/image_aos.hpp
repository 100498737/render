#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace render {

  struct ImageAOS {
    struct Pixel {
      std::uint8_t r{}, g{}, b{};
    };

    int width{}, height{};
    std::vector<Pixel> data;

    explicit ImageAOS(int w, int h) : width(w), height(h) {
      std::size_t const n = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
      data.assign(n, Pixel{0, 0, 0});
    }

    [[nodiscard]] std::size_t idx(int x, int y) const {
      return static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
             static_cast<std::size_t>(x);
    }

    void set(int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
      data[idx(x, y)] = Pixel{r, g, b};
    }

    void get(int x, int y, std::uint8_t & r, std::uint8_t & g, std::uint8_t & b) const {
      Pixel const & p = data[idx(x, y)];
      r               = p.r;
      g               = p.g;
      b               = p.b;
    }

    static std::uint8_t clamp01_to_u8(double v) {
      v            = std::clamp(v, 0.0, 1.0);
      int const vi = static_cast<int>(std::lround(v * 255.0));
      return static_cast<std::uint8_t>(std::clamp(vi, 0, 255));
    }

    void set01(int x, int y, double r, double g, double b) {
      set(x, y, clamp01_to_u8(r), clamp01_to_u8(g), clamp01_to_u8(b));
    }
  };

}  // namespace render
