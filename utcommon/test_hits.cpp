#include <gtest/gtest.h>
#include "render/vector.hpp"
#include "render/ray.hpp"
#include "render/hits.hpp"

namespace {
  inline bool approx(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
  }
}

// Casos de prueba para render::hit_sphere(ray, center, radius, tmin, tmax, &t, &n)

TEST(hits_sphere, miss_no_intersection) {
  render::ray r({0,0,0}, {1,0,0});          // apunta en +X
  render::vector c{0,0,-5};                  // esfera lejos en -Z
  double t{}; render::vector n{};
  bool hit = render::hit_sphere(r, c, /*radius*/1.0, /*tmin*/1e-3, /*tmax*/1e9, &t, &n);
  EXPECT_FALSE(hit);
}

TEST(hits_sphere, hit_front_face_basic) {
  render::ray r({0,0,0}, {0,0,-1});          // hacia -Z
  render::vector c{0,0,-5};                  // centro a 5 unidades
  double t{}; render::vector n{};
  bool hit = render::hit_sphere(r, c, 1.0, 1e-3, 1e9, &t, &n);
  ASSERT_TRUE(hit);
  // impactos en t = 4 y t = 6; debe elegir t=4
  EXPECT_NEAR(t, 4.0, 1e-9);
  // Punto impacto = (0,0,-4), normal = (p - c)/r = (0,0,1)
  EXPECT_NEAR(n.x, 0.0, 1e-12);
  EXPECT_NEAR(n.y, 0.0, 1e-12);
  EXPECT_NEAR(n.z, 1.0, 1e-12);
}

TEST(hits_sphere, tmin_gates_the_near_hit) {
  render::ray r({0,0,0}, {0,0,-1});
  render::vector c{0,0,-5};
  double t{}; render::vector n{};
  // Subimos tmin por encima del primer impacto (4): debe devolver el segundo (6).
  bool hit = render::hit_sphere(r, c, 1.0, /*tmin*/5.0, /*tmax*/1e9, &t, &n);
  ASSERT_TRUE(hit);
  EXPECT_NEAR(t, 6.0, 1e-9);
  // Punto impacto = (0,0,-6), normal = (0,0,-1)
  EXPECT_NEAR(n.z, -1.0, 1e-12);
}

TEST(hits_sphere, ray_starts_inside_sphere) {
  // Origen en el centro: estamos dentro; debe devolver la salida (t = 1 si dir es unitario)
  render::ray r({0,0,-5}, {0,0,-1});   // dentro de esfera (centro 0,0,-5, radio 1)
  render::vector c{0,0,-5};
  double t{}; render::vector n{};
  bool hit = render::hit_sphere(r, c, 1.0, 1e-3, 1e9, &t, &n);
  ASSERT_TRUE(hit);
  EXPECT_NEAR(t, 1.0, 1e-9);
  EXPECT_NEAR(n.x, 0.0, 1e-12);
  EXPECT_NEAR(n.y, 0.0, 1e-12);
  EXPECT_NEAR(n.z, -1.0, 1e-12);
}
