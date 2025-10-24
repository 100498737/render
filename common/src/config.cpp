#include "render/config.hpp"

namespace render {

  std::optional<Config> try_parse_config(std::string const & /*path*/, std::string * err) {
    if (err != nullptr) {
      *err = "";
    }

    Config cfg{
      .width             = 400,
      .height            = 225,
      .vfov_deg          = 60.0,
      .lookfrom          = Vec3{0.0, 0.0, 1.0},
      .lookat            = Vec3{0.0, 0.0, 0.0},
      .vup               = Vec3{0.0, 1.0, 0.0},
      .samples_per_pixel = 4,
      .seed              = 1'234ULL,
    };

    return cfg;  // siempre éxito de momento
  }

}  // namespace render
