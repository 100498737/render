#include "common/vector.hpp"
#include <gtest/gtest.h>

using common::Vec3;

TEST(vector, magnitude_zero) {
  Vec3 vec(0.0, 0.0, 0.0);
  EXPECT_DOUBLE_EQ(vec.magnitude(), 0.0);
}

TEST(vector, magnitude_positive) {
  Vec3 vec(3.0, 4.0, 0.0);
  EXPECT_DOUBLE_EQ(vec.magnitude(), 5.0);
}
