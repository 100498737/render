#include <cstdint>
#include <cstdio>
#include <optional>
#include <print>
#include <string>

#include "render/camera.hpp"
#include "render/config.hpp"
#include "render/image_aos.hpp"
#include "render/parser.hpp"
#include "render/ppm.hpp"
#include "render/scene.hpp"
#include "render/vector.hpp"

namespace {

  [[nodiscard]] int handle_bad_argc(int provided_args) {
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  std::string err_cfg;
  auto cfg = render::try_parse_config(argv[1], &err_cfg);
  if (!cfg) {
    std::println(stderr, "{}", err_cfg);
    return 1;
  }

  std::string err_scn;
  auto scn = render::try_parse_scene(argv[2], &err_scn);
  if (!scn) {
    std::println(stderr, "{}", err_scn);
    return 1;
  }

  int const W = static_cast<int>(cfg->width);
  int const H = static_cast<int>(cfg->height);
  render::ImageAOS img(W, H);

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double const rx = (W > 1) ? double(x) / double(W - 1) : 0.0;
      double const ry = (H > 1) ? double(y) / double(H - 1) : 0.0;
      img.set01(x, y, rx, ry, 0.25);
    }
  }

  bool const ok =
      render::write_ppm_gamma(argv[3], W, H, [&](int x, int y, double & r, double & g, double & b) {
        std::uint8_t R, G, B;
        img.get(x, y, R, G, B);
        r = R / 255.0;
        g = G / 255.0;
        b = B / 255.0;
      });

  if (!ok) {
    std::println(stderr, "Error: cannot write '{}'", argv[3]);
    return 1;
  }
  std::println(stderr, "OK: wrote {}", argv[3]);
  return 0;
}
