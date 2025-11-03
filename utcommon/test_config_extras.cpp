#include "render/config.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <string>

// Forward correcto dentro del namespace real
namespace render {

  std::optional<Config> try_parse_config(std::string const & filename, std::string * err);

}

using namespace render;  // opcional

static std::string write_cfg(std::string const & txt) {
  std::string p = "/tmp/cfg_extras.txt";
  std::ofstream f(p);
  f << txt;
  return p;
}

TEST(ConfigParse, NewKeysHappy) {
  std::string cfg = "image_width 300\n"
                    "aspect_ratio 1.5\n"
                    "field_of_view 45\n"
                    "camera_position 1 2 3\n"
                    "camera_target 0 0 0\n"
                    "camera_north 0 1 0\n"
                    "samples 4\n"
                    "gamma 2.2\n";
  auto path       = write_cfg(cfg);
  std::string err;
  auto c = try_parse_config(path, &err);
  ASSERT_TRUE(c.has_value()) << err;
  EXPECT_EQ(c->width, 300u);
  EXPECT_EQ(c->height, 200u);  // 300 / 1.5 = 200
  EXPECT_DOUBLE_EQ(c->vertical_fov_deg, 45.0);
  EXPECT_DOUBLE_EQ(c->lookfrom.x, 1.0);
  EXPECT_DOUBLE_EQ(c->lookfrom.y, 2.0);
  EXPECT_DOUBLE_EQ(c->lookfrom.z, 3.0);
  EXPECT_DOUBLE_EQ(c->gamma, 2.2);
}
