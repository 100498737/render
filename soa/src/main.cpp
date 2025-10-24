#include <cstdio>
#include <print>
#include <string>

#include "render/config.hpp"
#include "render/vector.hpp"

namespace {

  [[nodiscard]] int handle_bad_argc(int provided_args) {
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  std::string const config_path = argv[1];
  std::string const scene_path  = argv[2];
  std::string const out_path    = argv[3];
  (void) scene_path;
  (void) out_path;

  std::string err;
  auto cfg_opt = render::try_parse_config(config_path, &err);
  if (!cfg_opt) {
    std::println(stderr, "Error: {}", err);
    return 1;
  }

  render::Config const & cfg = *cfg_opt;

  std::println("Starting AOS rendering");
  std::println("Image {}x{}, vfov {} deg, spp {}", cfg.width, cfg.height, cfg.vfov_deg,
               cfg.samples_per_pixel);

  render::Vec3 v{1.0, 2.0, 3.0};
  std::println("Vector magnitude: {}", v.magnitude());

  return 0;
}
