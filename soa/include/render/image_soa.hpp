#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>

namespace render {

struct ImageSOA {
  int width{}, height{};
  std::vector<std::uint8_t> R, G, B;

  ImageSOA(int w, int h)
  : width(w), height(h), R(w*h,0), G(w*h,0), B(w*h,0) {}

  inline int idx(int x,int y) const { return y*width + x; }

  void set(int x,int y, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    const int i = idx(x,y);
    R[i]=r; G[i]=g; B[i]=b;
  }
  void get(int x,int y, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) const {
    const int i = idx(x,y);
    r = R[i]; g = G[i]; b = B[i];
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
