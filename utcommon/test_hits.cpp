#include "render/hits.hpp"
#include "render/ray.hpp"
#include "render/vector.hpp"
#include <gtest/gtest.h>

namespace {

  inline bool approx(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
  }

}  // namespace

//
// Casos de prueba para render::hit_sphere(ray, center, radius, tmin, tmax, &t, &n)
//
// Convención esperada (ya implementada en tu libcommon):
//  - Devuelve true si hay intersección con t en [tmin, tmax]
//  - t: primer impacto válido
//  - n: normal en el punto ( (p - center)/radius ), sin forzar “front-face”
//

TEST(hits_sphere, miss_no_intersection) {
  render::ray r({0, 0, 0}, {1, 0, 0});  // apunta en +X
  render::vector c{0, 0, -5};           // esfera lejos en -Z
  double t{};
  render::vector n{};
  bool hit = render::hit_sphere(r, c, /*radius*/ 1.0, /*tmin*/ 1e-3, /*tmax*/ 1e9, &t, &n);
  EXPECT_FALSE(hit);
}

TEST(hits_sphere, hit_front_face_basic) {
  render::ray r({0, 0, 0}, {0, 0, -1});  // hacia -Z
  render::vector c{0, 0, -5};            // centro a 5 unidades
  double t{};
  render::vector n{};
  bool hit = render::hit_sphere(r, c, 1.0, 1e-3, 1e9, &t, &n);
  ASSERT_TRUE(hit);
  // Solución analítica: impactos en t = 4 y t = 6; debe elegir t=4
  EXPECT_TRUE(approx(t, 4.0, 1e-9));
  // Punto impacto = (0,0,-4), normal = (p - c)/r = (0,0,1)
  EXPECT_TRUE(approx(n.x, 0.0));
  EXPECT_TRUE(approx(n.y, 0.0));
  EXPECT_TRUE(approx(n.z, 1.0));
}

TEST(hits_sphere, tmin_gates_the_near_hit) {
  render::ray r({0, 0, 0}, {0, 0, -1});
  render::vector c{0, 0, -5};
  double t{};
  render::vector n{};
  // Si subimos tmin por encima del primer impacto (4), el “cercano” queda descartado.
  bool hit = render::hit_sphere(r, c, 1.0, /*tmin*/ 5.0, /*tmax*/ 1e9, &t, &n);
  ASSERT_TRUE(hit);
  // Ahora debería devolver el segundo (6).
  EXPECT_TRUE(approx(t, 6.0, 1e-9));
  // Punto impacto = (0,0,-6), normal = (0,0,-1)
  EXPECT_TRUE(approx(n.z, -1.0));
}

TEST(hits_sphere, ray_starts_inside_sphere) {
  // Origen en el centro: estamos dentro; debe devolver la salida (t = radius si dir es unitario)
  render::ray r({0, 0, -5}, {0, 0, -1});  // dentro de esfera (centro 0,0,-5, radio 1)
  render::vector c{0, 0, -5};
  double t{};
  render::vector n{};
  bool hit = render::hit_sphere(r, c, 1.0, 1e-3, 1e9, &t, &n);
  ASSERT_TRUE(hit);
  EXPECT_TRUE(approx(t, 1.0, 1e-9));
  EXPECT_TRUE(approx(n.x, 0.0));
  EXPECT_TRUE(approx(n.y, 0.0));
  EXPECT_TRUE(approx(n.z, -1.0));
}

// ==== Tests de hit_cylinder (se apoyan en hits.hpp existente) ====
TEST(hits_cylinder, miss_no_intersection) {
  render::ray r({3, 0, 0}, {-1, 0, 0});
  render::vector base{0, 0, 0}, axis{0, 0, 1};
  double t{};
  render::vector n{};
  bool hit = render::hit_cylinder(r, base, axis, /*height*/ 4.0, /*radius*/ 1.0,
                                  /*tmin*/ 1e-3, /*tmax*/ 1e9, &t, &n);
  EXPECT_FALSE(hit);
}

TEST(hits_cylinder, hit_side_basic) {
  render::ray r({2, 0, 2}, {-1, 0, 0});
  render::vector base{0, 0, 0}, axis{0, 0, 1};
  double t{};
  render::vector n{};
  bool ok = render::hit_cylinder(r, base, axis, 4.0, 1.0, 1e-3, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_GT(t, 0.0);
  EXPECT_NEAR(std::abs(n.x), 1.0, 1e-6);
  EXPECT_NEAR(n.y, 0.0, 1e-6);
}

TEST(hits_cylinder, tmin_gates_near_side) {
  render::ray r({2, 0, 2}, {-1, 0, 0});
  render::vector base{0, 0, 0}, axis{0, 0, 1};
  double t{};
  render::vector n{};
  bool ok = render::hit_cylinder(r, base, axis, 4.0, 1.0, /*tmin*/ 2.0, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_GT(t, 2.0);
}

TEST(hits_cylinder, ray_starts_inside) {
  render::ray r({0.5, 0, 2}, {1, 0, 0});
  render::vector base{0, 0, 0}, axis{0, 0, 1};
  double t{};
  render::vector n{};
  bool ok = render::hit_cylinder(r, base, axis, 4.0, 1.0, 1e-3, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_GT(t, 0.0);
}
