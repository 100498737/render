#include <fstream>
#include <gtest/gtest.h>
#include <string>

#include "render/config.hpp"
#include "render/parser.hpp"
#include "render/vector.hpp"

using render::Config;

TEST(Config, OpensAndParsesMinimal) {
  char const * path = "tmp_cfg_ok.txt";
  std::ofstream cfg(path);
  cfg << "width 800\n"
      << "height 450\n"
      << "fov 60\n"
      << "samples 32\n"
      << "seed 1234\n"
      << "lookfrom 0 0 1\n"
      << "lookat 0 0 0\n"
      << "vup 0 1 0\n";
  cfg.close();

  std::string err;
  auto got = render::try_parse_config(path, &err);
  ASSERT_TRUE(got.has_value()) << err;

  EXPECT_EQ(got->width, 800u);
  EXPECT_EQ(got->height, 450u);
  EXPECT_NEAR(got->vertical_fov_deg, 60.0, 1e-12);
  EXPECT_EQ(got->samples_per_pixel, 32u);
  EXPECT_EQ(got->seed, 1'234u);

  EXPECT_DOUBLE_EQ(got->lookfrom.x, 0.0);
  EXPECT_DOUBLE_EQ(got->lookfrom.y, 0.0);
  EXPECT_DOUBLE_EQ(got->lookfrom.z, 1.0);

  EXPECT_DOUBLE_EQ(got->lookat.x, 0.0);
  EXPECT_DOUBLE_EQ(got->lookat.y, 0.0);
  EXPECT_DOUBLE_EQ(got->lookat.z, 0.0);

  EXPECT_DOUBLE_EQ(got->vup.x, 0.0);
  EXPECT_DOUBLE_EQ(got->vup.y, 1.0);
  EXPECT_DOUBLE_EQ(got->vup.z, 0.0);
}

TEST(Config, MissingFile) {
  std::string err;
  auto bad = render::try_parse_config("___nope___.cfg", &err);
  EXPECT_FALSE(bad.has_value());
  EXPECT_FALSE(err.empty());
}

TEST(Config, UnknownKey) {
  char const * path = "tmp_cfg_badkey.txt";
  std::ofstream(path) << "foo 42\n";
  std::string err;
  auto bad = render::try_parse_config(path, &err);
  EXPECT_FALSE(bad.has_value());
  EXPECT_NE(err.find("invalid key"), std::string::npos);
}

TEST(Config, InvalidValue) {
  char const * path = "tmp_cfg_badval.txt";
  std::ofstream(path) << "width abc\n";
  std::string err;
  auto bad = render::try_parse_config(path, &err);
  EXPECT_FALSE(bad.has_value());
  EXPECT_NE(err.find("invalid format"), std::string::npos);
}
