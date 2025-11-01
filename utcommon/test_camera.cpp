#include <gtest/gtest.h>

#include "render/camera.hpp"
#include "render/ray.hpp"
#include "render/vector.hpp"

namespace {

  // Compara dos doubles con tolerancia.
  bool approx(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
  }

  // Compara dos vectores con tolerancia.
  bool approx_vec(render::vector const & a, render::vector const & b, double eps = 1e-6) {
    return approx(a.x, b.x, eps) and approx(a.y, b.y, eps) and approx(a.z, b.z, eps);
  }

}  // namespace

// 1. La cámara guarda los parámetros básicos correctamente.
TEST(camera_basic, construct_camera) {
  std::uint32_t const width  = 400;
  std::uint32_t const height = 200;
  double const vfov          = 60.0;
  render::vector const lookfrom{0.0, 0.0, 1.0};
  render::vector const lookat{0.0, 0.0, 0.0};
  render::vector const vup{0.0, 1.0, 0.0};
  std::uint32_t const spp  = 4U;
  std::uint64_t const seed = 42ULL;

  render::camera cam(width, height, vfov, lookfrom, lookat, vup, spp, seed);

  // Comprobaciones "baratas"
  EXPECT_EQ(cam.image_width(), width);
  EXPECT_EQ(cam.image_height(), height);
  EXPECT_EQ(cam.spp(), spp);

  // El origen interno de la cámara debe coincidir con lookfrom.
  render::vector cam_o = cam.origin();
  EXPECT_TRUE(approx_vec(cam_o, lookfrom, 1e-12));
}

// 2. Un rayo desde el centro de la imagen debe mirar hacia la escena,
// aproximadamente hacia lookat - lookfrom.
TEST(camera_basic, center_ray_points_toward_lookat) {
  std::uint32_t const width  = 400;
  std::uint32_t const height = 200;
  double const vfov          = 60.0;
  render::vector const lookfrom{0.0, 0.0, 1.0};
  render::vector const lookat{0.0, 0.0, 0.0};
  render::vector const vup{0.0, 1.0, 0.0};
  std::uint32_t const spp  = 8U;
  std::uint64_t const seed = 1'234ULL;

  render::camera cam(width, height, vfov, lookfrom, lookat, vup, spp, seed);

  // Pixel "central"
  std::uint32_t const cx = width / 2U;
  std::uint32_t const cy = height / 2U;

  render::ray r = cam.get_ray(cx, cy, 0);

  // origen del rayo == lookfrom
  EXPECT_TRUE(approx_vec(r.origin, lookfrom, 1e-12));

  // dirección debería ir más o menos hacia lookat (0,0,0),
  // o sea (lookat - lookfrom) = (0,0,-1), normalizado.
  render::vector ideal_dir = (lookat - lookfrom).normalized();

  // Permitimos cierto error porque hay jitter subpixel.
  EXPECT_NEAR(r.direction.x, ideal_dir.x, 0.1);
  EXPECT_NEAR(r.direction.y, ideal_dir.y, 0.1);
  EXPECT_NEAR(r.direction.z, ideal_dir.z, 0.1);

  // Debería ser un vector unitario
  EXPECT_NEAR(r.direction.magnitude(), 1.0, 1e-6);

  // IMPORTANTE: como la cámara está en z=+1 mirando al origen (z=0),
  // la dirección debería tener z negativa (apunta "hacia delante").
  EXPECT_LT(r.direction.z, 0.0);
}

// 3. El jitter funciona: dos rayos distintos del MISMO píxel no deben ser idénticos.
//    (Si fallara este test es que no estamos aplicando jitter por muestra.)
TEST(camera_basic, jitter_changes_ray_direction) {
  std::uint32_t const width  = 128;
  std::uint32_t const height = 64;
  double const vfov          = 45.0;
  render::vector const lookfrom{0.0, 0.0, 1.0};
  render::vector const lookat{0.0, 0.0, 0.0};
  render::vector const vup{0.0, 1.0, 0.0};
  std::uint32_t const spp  = 16U;
  std::uint64_t const seed = 99ULL;

  render::camera cam(width, height, vfov, lookfrom, lookat, vup, spp, seed);

  // Elegimos un píxel cualquiera.
  std::uint32_t const px = 10;
  std::uint32_t const py = 20;

  render::ray r0 = cam.get_ray(px, py, 0);
  render::ray r1 = cam.get_ray(px, py, 1);

  // El origen siempre es el mismo
  EXPECT_TRUE(approx_vec(r0.origin, lookfrom, 1e-12));
  EXPECT_TRUE(approx_vec(r1.origin, lookfrom, 1e-12));

  // Pero las direcciones deberían diferir un poco.
  // (OJO: esto asume que get_ray consume RNG cada vez.)
  double dx = std::abs(r0.direction.x - r1.direction.x);
  double dy = std::abs(r0.direction.y - r1.direction.y);
  double dz = std::abs(r0.direction.z - r1.direction.z);

  double diff = dx + dy + dz;

  EXPECT_GT(diff, 1e-6);  // debería haber alguna diferencia real
}

TEST(Camera, CenterRayPointsNearLookAt_Basic) {
  std::uint32_t const W = 100, H = 80;
  render::camera cam{
    W,
    H,
    /*vfov_deg*/ 40.0,
    /*from*/ {0, 0, 1},
    /*at*/
    {0, 0, 0},
    /*vup*/
    {0, 1, 0},
    /*spp*/
    1U,
    /*seed*/ 1'234ULL,
    /*aperture*/ 0.0,
    /*focus*/ 1.0
  };

  std::uint32_t const cx = W / 2, cy = H / 2;
  render::ray r = cam.get_ray(cx, cy, 0);

  render::vector want =
      (render::vector{0, 0, 0} - render::vector{0, 0, 1}).normalized();  // (0,0,-1)
  double cosang = r.direction.dot(want);                                 // ambas unitarias
  EXPECT_GT(cosang, 0.995) << "El rayo central debe apuntar ~al lookat";
}

TEST(Camera, Jitter_IsDeterministic_WithSeed) {
  using namespace render;
  camera c1{
    64, 64, 40.0, {0, 0, 1},
       {0, 0, 0},
       {0, 1, 0},
       4u, 1'234ull, 0.0, 1.0
  };
  camera c2{
    64, 64, 40.0, {0, 0, 1},
       {0, 0, 0},
       {0, 1, 0},
       4u, 1'234ull, 0.0, 1.0
  };
  auto r1 = c1.get_ray(10, 20, 0);
  auto r2 = c2.get_ray(10, 20, 0);
  EXPECT_NEAR(r1.direction.x, r2.direction.x, 1e-12);
  EXPECT_NEAR(r1.direction.y, r2.direction.y, 1e-12);
  EXPECT_NEAR(r1.direction.z, r2.direction.z, 1e-12);
}

TEST(Camera, DOF_Path_Executes) {
  render::camera cam{
    32,
    24,
    40.0,
    /*from*/ {0, 0, 1},
    /*at*/
    {0, 0, 0},
    /*vup*/
    {0, 1, 0},
    /*spp*/
    2u,
    /*seed*/ 7ull,
    /*aperture*/ 0.3,
    /*focus*/ 1.0
  };
  auto r0 = cam.get_ray(10, 10, 0);
  auto r1 = cam.get_ray(10, 10, 1);
  // Con DOF y spp>1, alguna diferencia debe existir
  double ddot = r0.direction.dot(r1.direction);
  EXPECT_LT(ddot, 0.999999);  // no exactamente iguales
}

TEST(Camera, CornerRays_ExecuteBranches) {
  render::camera cam{
    64, 48, 120.0, {1, 1, 2},
       {0, 0, 0},
       {0, 1, 0},
       1u, 42ull, 0.0, 1.0
  };
  (void) cam.get_ray(0, 0, 0);
  (void) cam.get_ray(63, 47, 0);
}
