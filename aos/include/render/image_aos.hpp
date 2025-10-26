#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>

namespace render {

struct ImageAOS {
  struct Pixel { std::uint8_t r{}, g{}, b{}; };
  int width{}, height{};
  std::vector<Pixel> data;

  ImageAOS(int w, int h) : width(w), height(h), data(w*h) {}

  inline int idx(int x,int y) const { return y*width + x; }

  void set(int x,int y, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    data[idx(x,y)] = Pixel{r,g,b};
  }
  void get(int x,int y, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) const {
    const Pixel& p = data[idx(x,y)];
    r=p.r; g=p.g; b=p.b;
  }
  static std::uint8_t clamp01_to_u8(double v) {
    v = std::clamp(v, 0.0, 1.0);
    int vi = (int)std::lround(v*255.0);
    return (std::uint8_t)std::clamp(vi, 0, 255);
  }
  void set01(int x,int y, double r, double g, double b) {
    set(x,y, clamp01_to_u8(r), clamp01_to_u8(g), clamp01_to_u8(b));
  }
};

} // namespace render
