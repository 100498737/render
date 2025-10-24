#include <gtest/gtest.h>
#include <fstream>
#include "render/config.hpp"

using render::try_parse_config;
using render::Config;

TEST(Config, OpensAndParsesMinimal) {
  // Creamos un cfg temporal
  const char* path = "tmp_cfg_ok.txt";
  std::ofstream(path)
    << "# minimal cfg\n"
    << "image_width=800\n"
    << "aspect_ratio=1.7777777778\n"
    << "samples_per_pixel=32\n"
    << "max_depth=8\n"
    << "gamma=2.2\n";

  std::string err;
  auto got = try_parse_config(path, &err);
  ASSERT_TRUE(got.has_value()) << err;
  EXPECT_EQ(got->image_width, 800);
  EXPECT_NEAR(got->aspect_ratio, 1.7777777778, 1e-9);
  EXPECT_EQ(got->samples_per_pixel, 32);
  EXPECT_EQ(got->max_depth, 8);
  EXPECT_NEAR(got->gamma, 2.2, 1e-12);
}

TEST(Config, MissingFile) {
  std::string err;
  auto bad = try_parse_config("___nope___.cfg", &err);
  EXPECT_FALSE(bad.has_value());
  EXPECT_FALSE(err.empty());
}

TEST(Config, UnknownKey) {
  const char* path = "tmp_cfg_badkey.txt";
  std::ofstream(path) << "foo=42\n";
  std::string err;
  auto bad = try_parse_config(path, &err);
  EXPECT_FALSE(bad.has_value());
  EXPECT_NE(err.find("unknown key"), std::string::npos);
}

TEST(Config, InvalidValue) {
  const char* path = "tmp_cfg_badval.txt";
  std::ofstream(path) << "image_width=abc\n";
  std::string err;
  auto bad = try_parse_config(path, &err);
  EXPECT_FALSE(bad.has_value());
  EXPECT_NE(err.find("invalid"), std::string::npos);
}
