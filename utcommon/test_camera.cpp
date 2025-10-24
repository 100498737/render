#include <gtest/gtest.h>

#include "render/camera.hpp"
#include "render/ray.hpp"
#include "render/vector.hpp"

namespace {

  // helper para comprobar que dos doubles son casi iguales
  bool approx(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
  }

  // helper para comparar dos vectores (sin necesidad de == estricta)
  bool approx_vec(render::vector const & a, render::vector const & b, double eps = 1e-6) {
    return approx(a.x, b.x, eps) && approx(a.y, b.y, eps) && approx(a.z, b.z, eps);
  }

}  // namespace

// -------------------------------------------------------------
// TEST 1: la cámara guarda correctamente los parámetros básicos
// -------------------------------------------------------------
TEST(camera_basic, construct_camera) {
  std::uint32_t const width  = 400;
  std::uint32_t const height = 200;
  double const vfov_deg      = 60.0;

  render::vector lookfrom{0.0, 0.0, 1.0};
  render::vector lookat{0.0, 0.0, 0.0};
  render::vector vup{0.0, 1.0, 0.0};

  std::uint32_t const spp  = 4;
  std::uint64_t const seed = 1'234ULL;

  render::camera cam(width, height, vfov_deg, lookfrom, lookat, vup, spp, seed);

  // Comprobar que spp() devuelve lo que le pasamos
  EXPECT_EQ(cam.spp(), spp);
}

// -------------------------------------------------------------
// TEST 2: get_ray devuelve un rayo con origen en la cámara
// -------------------------------------------------------------
TEST(camera_basic, get_ray_origin_matches_camera) {
  render::camera cam(400, 200, 60.0, render::vector{0.0, 0.0, 1.0}, render::vector{0.0, 0.0, 0.0},
                     render::vector{0.0, 1.0, 0.0}, 4,
                     9'999ULL  // semilla conocida
  );

  // Pedimos un rayo de un píxel concreto
  render::ray r = cam.get_ray(100u, 50u, 0u);

  // El origen del rayo tiene que ser (lookfrom), que es (0,0,1)
  render::vector expected_origin{0.0, 0.0, 1.0};
  EXPECT_TRUE(approx_vec(r.origin, expected_origin));
}

// -------------------------------------------------------------
// TEST 3: el rayo debe mirar más o menos hacia -Z (escena delante)
// -------------------------------------------------------------
TEST(camera_basic, get_ray_points_forward) {
  render::camera cam(400, 200, 60.0,
                     render::vector{0.0, 0.0, 1.0},  // cámara está en z=1 mirando al origen
                     render::vector{0.0, 0.0, 0.0}, render::vector{0.0, 1.0, 0.0}, 4, 42ULL);

  render::ray r = cam.get_ray(200u, 100u, 0u);

  // dirección normalizada => debería apuntar "más o menos" hacia (0,0,-1)
  // o sea, z negativa, y longitud aprox 1
  EXPECT_LT(r.direction.z, 0.0);  // z < 0 => apunta hacia delante
  EXPECT_NEAR(r.direction.magnitude(), 1.0, 1e-6);
}
