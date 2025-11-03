#include "render/ppm.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace render;

TEST(PPM, LegacyAndGammaOverload) {
  auto sampler = [](int, int, double & r, double & g, double & b) {
    r = 0.25;
    g = 0.5;
    b = 0.75;
  };
  // Legacy (4 args)
  EXPECT_FALSE(write_ppm_gamma("/this/dir/should/not/exist/out.ppm", 2, 2, sampler));  // open fail
  EXPECT_TRUE(write_ppm_gamma("/tmp/ok_legacy.ppm", 2, 2, sampler));
  EXPECT_TRUE(write_ppm_gamma("/tmp/ok_gamma22.ppm", 2, 2, 2.2, sampler));
  EXPECT_TRUE(write_ppm_gamma("/tmp/ok_gamma10.ppm", 2, 2, 1.0, sampler));
}

TEST(PPM, RejectZeroDims) {
  auto sampler = [](int, int, double & r, double & g, double & b) { r = g = b = 0.0; };
  EXPECT_FALSE(write_ppm_gamma("out/build/coverage/zero.ppm", 0, 0, sampler));
}
