#include "render/ray.hpp"
#include "render/vector.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(test_vector_basics, addition_and_subtraction) {
  render::vector a{1.0, 2.0, 3.0};
  render::vector b{0.5, -1.0, 4.0};

  render::vector sum = a + b;
  EXPECT_DOUBLE_EQ(sum.x, 1.5);
  EXPECT_DOUBLE_EQ(sum.y, 1.0);
  EXPECT_DOUBLE_EQ(sum.z, 7.0);

  render::vector diff = a - b;
  EXPECT_DOUBLE_EQ(diff.x, 0.5);
  EXPECT_DOUBLE_EQ(diff.y, 3.0);
  EXPECT_DOUBLE_EQ(diff.z, -1.0);
}

TEST(test_vector_basics, dot_product) {
  render::vector a{1.0, 2.0, 3.0};
  render::vector b{4.0, -5.0, 6.0};

  double d = a.dot(b);
  // dot = 1*4 + 2*(-5) + 3*6 = 4 -10 +18 = 12
  EXPECT_DOUBLE_EQ(d, 12.0);
}

TEST(test_vector_basics, cross_product) {
  render::vector a{1.0, 0.0, 0.0};
  render::vector b{0.0, 1.0, 0.0};

  render::vector c = a.cross(b);
  // x×y = z
  EXPECT_DOUBLE_EQ(c.x, 0.0);
  EXPECT_DOUBLE_EQ(c.y, 0.0);
  EXPECT_DOUBLE_EQ(c.z, 1.0);
}

TEST(test_vector_basics, magnitude_and_normalized) {
  render::vector a{3.0, 4.0, 0.0};

  EXPECT_DOUBLE_EQ(a.magnitude(), 5.0);

  render::vector n = a.normalized();
  // norma(n) debe ser 1 (tolerancia pequeña)
  double len_n = n.magnitude();
  EXPECT_NEAR(len_n, 1.0, 1e-12);

  // dirección debe ser la misma dirección que 'a'
  EXPECT_NEAR(n.x, 3.0 / 5.0, 1e-12);
  EXPECT_NEAR(n.y, 4.0 / 5.0, 1e-12);
  EXPECT_NEAR(n.z, 0.0, 1e-12);
}

TEST(test_ray_basic, at_parameter) {
  render::vector origin{1.0, 2.0, 3.0};
  render::vector dir{0.0, 1.0, 0.0};

  render::ray r(origin, dir);

  // r.at(t) = origin + dir * t
  render::vector p0 = r.at(0.0);
  EXPECT_DOUBLE_EQ(p0.x, 1.0);
  EXPECT_DOUBLE_EQ(p0.y, 2.0);
  EXPECT_DOUBLE_EQ(p0.z, 3.0);

  render::vector p1 = r.at(5.0);
  EXPECT_DOUBLE_EQ(p1.x, 1.0);
  EXPECT_DOUBLE_EQ(p1.y, 7.0);  // 2.0 + 1.0 * 5.0
  EXPECT_DOUBLE_EQ(p1.z, 3.0);
}
