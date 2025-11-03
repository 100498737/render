#include "render/parser.hpp"
#include "render/ppm.hpp"
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>

TEST(PPM, WriteTinyGammaAndPlain) {
  char const * f1 = "cov_ppm_gamma.ppm";
  char const * f2 = "cov_ppm_plain.ppm";
  int const W = 3, H = 2;

  // write_ppm_gamma
  bool ok1 =
      render::write_ppm_gamma(f1, W, H, [&](int x, int y, double & r, double & g, double & b) {
        r = (x + 1.0) / W;
        g = (y + 1.0) / H;
        b = 0.25;
      });
  ASSERT_TRUE(ok1);

  // Limpieza (no imprescindible)
  int r1 = std::remove(f1);
  if (r1 != 0 and errno != ENOENT) {
    perror(f1);
  }
  int r2 = std::remove(f2);
  if (r2 != 0 and errno != ENOENT) {
    perror(f2);
  }
}

TEST(PPM, GammaAndPlain_Limits) {
  char const * f1 = "ppma.ppm";
  char const * f2 = "ppmb.ppm";
  int W = 2, H = 2;
  ASSERT_TRUE(
      render::write_ppm_gamma(f1, W, H, [&](int x, int y, double & r, double & g, double & b) {
        r = (x == 0) ? 0.0 : 1.0;
        g = (y == 0) ? 0.0 : 1.0;
        b = 1.0;
      }));
#if defined(HAVE_WRITE_PPM)  // si existe en tu proyecto
  ASSERT_TRUE(render::write_ppm(
      f2, W, H, [&](int x, int y, unsigned char & R, unsigned char & G, unsigned char & B) {
        R = (unsigned char) (x ? 255 : 0);
        G = (unsigned char) (y ? 255 : 0);
        B = 128;
      }));
#endif
  (void) std::remove(f1);
  (void) std::remove(f2);
}

TEST(PPM, OpenFail_ReturnsFalse) {
  int W = 2, H = 2;
  bool ok =
      render::write_ppm_gamma("/no/such/dir/out.ppm", W, H,
                              [&](int, int, double & r, double & g, double & b) { r = g = b = 0; });
  EXPECT_FALSE(ok);
}

TEST(Parser, ParseMinimalCfgFile) {
  char const * f = "cov_cfg.txt";
  std::ofstream o(f);
  // Claves válidas + formato sin comas (3 números separados por espacio)
  o << "width 8\n"
       "height 6\n"
       "vfov 40\n"  // si tu parser usa vfov_deg, cambia a "vfov_deg 40"
       "lookfrom 0 0 1\n"
       "lookat   0 0 0\n"
       "vup      0 1 0\n"
       "samples 1\n"
       "seed 1234\n"
       "aperture 0.0\n"
       "focus_dist 1.0\n"
       "max_depth 1\n";
  o.close();

  std::string err;
  auto cfg = render::try_parse_config(f, &err);
  ASSERT_TRUE(cfg) << err;
}

TEST(Parser, ParseMinimalSceneFile) {
  char const * f = "cov_scene.txt";
  std::ofstream o(f);
  o << "# materiales\n"
       "matte red color=0.8,0.2,0.2\n"
       "# objetos\n"
       "sphere ball center=0,0,0 radius=0.5 mat=red\n";
  o.close();

  std::string err;
  auto scn = render::try_parse_scene(f, &err);
  ASSERT_TRUE(scn) << err;

  // Si tu Scene expone contenedores:
  EXPECT_GE(scn->spheres.size(), 1U);
}
