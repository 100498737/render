#include <algorithm>  // std::clamp
#include <cstdint>
#include <cstdio>
#include <cstdlib>  // std::getenv
#include <optional>
#include <print>
#include <string>

#include "render/camera.hpp"
#include "render/config.hpp"
#include "render/hits.hpp"
#include "render/image_aos.hpp"
#include "render/parser.hpp"
#include "render/ppm.hpp"
#include "render/ray.hpp"
#include "render/scene.hpp"
#include "render/vector.hpp"

static void trace_pixel(render::camera & cam, render::Scene const & scn, int x, int y,
                        int max_depth, double & r, double & g, double & b) {
  (void) max_depth;  // lo usaremos cuando haya rebotes

  // SPP desde env (RENDER_SPP) o por defecto 4
  auto get_spp = []() -> int {
    if (char const * s = std::getenv("RENDER_SPP")) {
      int v = std::atoi(s);
      return v > 0 ? v : 1;
    }
    return 4;
  };
  int const SPP = get_spp();

  double acc_r = 0.0, acc_g = 0.0, acc_b = 0.0;

  for (int s = 0; s < SPP; ++s) {
    render::ray ray = cam.get_ray(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y),
                                  static_cast<std::uint32_t>(s));

    // Escaneo de escena
    double t_closest = 1e9;
    render::vector n{};
    bool hit = false;

    // Esferas
    for (auto const & sph : scn.spheres) {
      double t;
      render::vector nn;
      if (render::hit_sphere(ray, sph.center, sph.radius, 1e-6, t_closest, &t, &nn)) {
        t_closest = t;
        n         = nn;
        hit       = true;
      }
    }
    // Cilindros (tu firma: height antes que radius)
    for (auto const & cyl : scn.cylinders) {
      double t;
      render::vector nn;
      if (render::hit_cylinder(ray, cyl.base, cyl.axis, cyl.height, cyl.radius, 1e-6, t_closest, &t,
                               &nn))
      {
        t_closest = t;
        n         = nn;
        hit       = true;
      }
    }

    if (hit) {
      // Color por normal (map [-1,1] -> [0,1])
      render::vector c = n * 0.5 + render::vector{0.5, 0.5, 0.5};
      acc_r += c.x;
      acc_g += c.y;
      acc_b += c.z;
    } else {
      // Cielo
      double const t = 0.5 * (ray.direction.y + 1.0);
      acc_r += (1.0 - t) * 1.0 + t * 0.5;
      acc_g += (1.0 - t) * 1.0 + t * 0.7;
      acc_b += (1.0 - t) * 1.0 + t * 1.0;
    }
  }

  double const inv = 1.0 / static_cast<double>(SPP);
  r                = std::clamp(acc_r * inv, 0.0, 1.0);
  g                = std::clamp(acc_g * inv, 0.0, 1.0);
  b                = std::clamp(acc_b * inv, 0.0, 1.0);
}

namespace {

  [[nodiscard]] int handle_bad_argc(int provided_args) {
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

static double envd(char const * k, double def) {
  if (char const * s = std::getenv(k)) {
    return std::atof(s);
  }
  return def;
}

static int envi(char const * k, int def) {
  if (char const * s = std::getenv(k)) {
    int v = std::atoi(s);
    return v > 0 ? v : def;
  }
  return def;
}

static render::vector envv3(char const * k, render::vector def) {
  if (char const * s = std::getenv(k)) {
    double x = def.x, y = def.y, z = def.z;
    std::sscanf(s, "%lf,%lf,%lf", &x, &y, &z);
    return {x, y, z};
  }
  return def;
}

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

  std::println(stderr, "scene: {} spheres, {} cylinders", scn->spheres.size(),
               scn->cylinders.size());
  if (!scn->spheres.empty()) {
    auto const & s = scn->spheres.front();
    std::println(stderr, "first sphere: c=({}, {}, {}), r={}", s.center.x, s.center.y, s.center.z,
                 s.radius);
  }

  double vfov_deg  = envd("RENDER_VFOV", 40.0);
  auto from        = envv3("RENDER_FROM", {0, 0, 1});
  auto at          = envv3("RENDER_AT", {0, 0, 0});
  auto vup         = envv3("RENDER_VUP", {0, 1, 0});
  unsigned spp_cam = static_cast<unsigned>(envi("RENDER_SPP", 1));  // para RNG de cámara
  unsigned seed    = static_cast<unsigned>(envi("RENDER_SEED", 1'234));
  double aperture  = envd("RENDER_APERTURE", 0.0);
  double focus     = envd("RENDER_FOCUS", 1.0);

  render::camera cam{cfg->width, cfg->height, vfov_deg, from,     at,
                     vup,        spp_cam,     seed,     aperture, focus};

  // DEBUG: imprime valores efectivos para comprobar que llegan
  std::println(stderr,
               "cam from=({}, {}, {}), at=({}, {}, {}), vup=({}, {}, {}), vfov={}, aperture={}, "
               "focus={}, spp={}",
               from.x, from.y, from.z, at.x, at.y, at.z, vup.x, vup.y, vup.z, vfov_deg, aperture,
               focus, spp_cam);

  int const W = static_cast<int>(cfg->width);
  int const H = static_cast<int>(cfg->height);
  render::ImageAOS img(W, H);

  {
    int cx = W / 2, cy = H / 2;
    render::ray ray = cam.get_ray((uint32_t) cx, (uint32_t) cy, 0u);
    double t;
    render::vector n;
    bool any = (!scn->spheres.empty() &&
                render::hit_sphere(ray, scn->spheres.front().center, scn->spheres.front().radius,
                                   1e-6, 1e9, &t, &n));
    std::println(stderr, "center-pixel hit? {}  t={}", any, any ? t : -1.0);
  }

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      double r01, g01, b01;
      trace_pixel(cam, *scn, x, y, /*max_depth*/ 5, r01, g01, b01);
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
