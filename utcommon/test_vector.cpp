#include "render/vector.hpp"
#include <gtest/gtest.h>

TEST(test_vector_magnitude_zero, BasicAssertions) {
  render::vector vec{0.0, 0.0, 0.0};
  EXPECT_EQ(vec.magnitude(), 0.0);
}

TEST(test_vector_magnitude_positive, BasicAssertions) {
  render::vector vec{3.0, 4.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 5.0);
}
