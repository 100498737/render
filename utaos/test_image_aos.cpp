#include <gtest/gtest.h>
#include "render/image_aos.hpp"

TEST(image_aos_basic, set_get_u8) {
  render::ImageAOS img(4,3);
  img.set(1,2, 100,150,200);
  std::uint8_t r,g,b; img.get(1,2,r,g,b);
  EXPECT_EQ(r,100); EXPECT_EQ(g,150); EXPECT_EQ(b,200);
}

TEST(image_aos_basic, set01_clamps) {
  render::ImageAOS img(2,1);
  img.set01(1,0, 0.0, 1.0, 0.25);
  std::uint8_t r,g,b; img.get(1,0,r,g,b);
  EXPECT_EQ(r,0); EXPECT_EQ(g,255); EXPECT_EQ(b,64);
}
