#pragma once
#include <optional>
#include <string>

namespace render {

struct Config {
  int    image_width       = 256;
  double aspect_ratio      = 16.0/9.0;
  int    samples_per_pixel = 10;
  int    max_depth         = 10;
  double gamma             = 2.2;
  // TODO: cámara, semillas, etc.
};

std::optional<Config> try_parse_config(const std::string& path, std::string* err);

} // namespace render
