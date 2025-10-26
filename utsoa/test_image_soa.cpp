#include <gtest/gtest.h>
#include "render/image_soa.hpp"

TEST(image_soa_basic, set_get_u8) {
  render::ImageSOA img(4,3);
  img.set(2,1, 10,20,30);
  std::uint8_t r,g,b; img.get(2,1,r,g,b);
  EXPECT_EQ(r,10); EXPECT_EQ(g,20); EXPECT_EQ(b,30);
}

TEST(image_soa_basic, set01_clamps) {
  render::ImageSOA img(2,1);
  img.set01(0,0, -1.0, 0.5, 2.0);
  std::uint8_t r,g,b; img.get(0,0,r,g,b);
  EXPECT_EQ(r,0); EXPECT_EQ(g,128); EXPECT_EQ(b,255);
}
