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
#include "render/ray.hpp"
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

  // ... tras comprobar que cfg y scn son válidos:

  // AÑADIR (solo para comprobar que enlaza el ctor nuevo con DOF; no se usa aún)
  render::camera cam{
    cfg->width, cfg->height,
    /*vfov_deg*/ 40.0, render::vector{0, 0, 1}, // lookfrom
    render::vector{0, 0, 0}, // lookat
    render::vector{0, 1, 0}, // vup
    /*samples_per_pixel*/
    1u,
    /*seed*/ 1'234ull,
    /*aperture*/ 0.3, // 0 => pinhole (sin DOF)
    /*focus_dist*/ 1.5
  };

  int const W = static_cast<int>(cfg->width);
  int const H = static_cast<int>(cfg->height);
  render::ImageAOS img(W, H);

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      // 1 rayo por píxel (de momento)
      render::ray r = cam.get_ray(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y),
                                  /*sample_id*/ 0);

      // “Cielo” simple según la componente Y de la dirección
      // OJO: si tu ray usa .dir() en vez de .direction(), cámbialo a .dir()
      double const ydir = r.direction.y;
      double t          = 0.5 * (ydir + 1.0);         // mapear [-1,1] → [0,1]
      double r01        = (1.0 - t) * 1.0 + t * 0.5;  // blanco → azul claro
      double g01        = (1.0 - t) * 1.0 + t * 0.7;
      double b01        = (1.0 - t) * 1.0 + t * 1.0;

      img.set01(x, y, r01, g01, b01);
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
