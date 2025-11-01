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

// --- Esfera: da hit centrado ---
TEST(Hits, Sphere_Center_Hit) {
  using namespace render;
  vector C{0, 0, 0};
  double R = 0.5;
  ray r{
    /*O*/ {0, 0,  1}, /*D*/
    {0, 0, -1}
  };     // hacia el origen

  double t;
  vector n;
  bool ok = hit_sphere(r, C, R, 1e-6, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_NEAR(t, 0.5, 1e-6);  // 1 -> 0.5
  EXPECT_NEAR(n.x, 0.0, 1e-6);
  EXPECT_NEAR(n.y, 0.0, 1e-6);
  EXPECT_NEAR(n.z, 1.0, 1e-6);  // normal (0,0,1)
}

// --- Esfera: miss claro ---
TEST(Hits, Sphere_Clear_Miss) {
  using namespace render;
  vector C{0, 0, 0};
  double R = 0.4;
  ray r{
    /*O*/ {1.0, 0.0, 1.0}, /*D*/
    {  0,   0,  -1}
  };           // paralelo a Z, lejos en X

  double t;
  vector n;
  bool ok = hit_sphere(r, C, R, 1e-6, 1e9, &t, &n);
  EXPECT_FALSE(ok);
}

// --- Cilindro: tapa inferior y eje NO normalizado ---
TEST(Hits, Cylinder_BottomCap_WithNonUnitAxis) {
  using namespace render;
  vector base{0, 0, 0};
  vector axis{0, 2, 0};  // ← no normalizado a propósito
  double const height = 1.0;
  double const radius = 0.5;

  // Rayo vertical desde abajo: debe chocar la tapa inferior (y=0)
  ray r{
    /*O*/ {0, -1, 0}, /*D*/
    {0,  1, 0}
  };

  double t;
  vector n;
  bool ok = hit_cylinder(r, base, axis, height, radius, 1e-6, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_NEAR(t, 1.0, 1e-6);  // -1 -> 0
  // Normal hacia fuera en tapa inferior = -axis_unit = (0,-1,0)
  EXPECT_LT(n.y, -0.999);
}

TEST(Hits, Sphere_Tangent) {
  using namespace render;
  vector C{0, 0, 0};
  double R = 1.0;
  // Tangente en (0,1,0): y=1, z=0  ⇒ x=0
  ray r{
    {-2, 1, 0},
    { 1, 0, 0}
  };
  double t;
  vector n;
  bool ok = hit_sphere(r, C, R, 1e-6, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_NEAR(t, 2.0, 1e-6);  // -2 → 0
  EXPECT_NEAR(n.x, 0.0, 1e-6);
  EXPECT_NEAR(n.y, 1.0, 1e-6);
  EXPECT_NEAR(n.z, 0.0, 1e-6);
}

TEST(hits_cylinder, miss_no_intersection) {
  using namespace render;

  // Cilindro: eje Y, base inferior en y=0, altura=1, radio=0.5
  vector base{0, 0, 0};
  vector axis{0, 1, 0};  // se normaliza internamente en tu hit_cylinder
  double const height = 1.0;
  double const radius = 0.5;

  // Rayo paralelo al eje (0,1,0), pero DESPLAZADO en X más allá del radio.
  // ⇒ Lateral: nunca entra (distancia al eje = 1.0 > 0.5).
  // ⇒ Tapas: aunque cruce los planos y=0/1, la distancia radial al centro > radio, así que tampoco.
  ray ray_away{
    /*origin*/ {1.0, 0.5, 0.0}, /*dir*/
    {0.0, 1.0, 0.0}
  };

  double t;
  vector n;
  bool ok = hit_cylinder(ray_away, base, axis, height, radius,
                         /*t_min*/ 1e-6, /*t_max*/ 1e9, &t, &n);
  EXPECT_FALSE(ok) << "Ray is parallel to axis and outside radius: should miss";
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

TEST(Hits, CylinderLateral_TowardsX) {
  using namespace render;
  vector base{0, 0, 0}, axis{0, 1, 0};
  double const height = 1.0, radius = 0.5;

  // Desde la derecha mirando hacia -X: debe cortar el lateral en x=+0.5
  ray ray{
    { 2.0, 0.5, 0.0},
    {-1.0, 0.0, 0.0}
  };

  double t;
  vector n;
  bool ok = hit_cylinder(ray, base, axis, height, radius, 1e-6, 1e9, &t, &n);
  ASSERT_TRUE(ok);
  EXPECT_NEAR(t, 1.5, 1e-6);     // (2.0 -> 0.5) = 1.5
  EXPECT_NEAR(n.x, +1.0, 1e-3);  // normal ~ +X en ese punto
  EXPECT_NEAR(n.y, 0.0, 1e-3);
  EXPECT_NEAR(n.z, 0.0, 1e-3);
}
