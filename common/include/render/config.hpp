#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "render/vector.hpp"  // <- necesitamos Vec3 antes de usarlo

namespace render {

  struct Config {
    std::uint32_t width;
    std::uint32_t height;
    double vfov_deg;  // vertical FOV en grados
    Vec3 lookfrom;
    Vec3 lookat;
    Vec3 vup;
    std::uint32_t samples_per_pixel;
    std::uint64_t seed;
  };

  // stub temporal: intenta "parsear" el fichero de config.
  // Devuelve Config fijo por ahora, y pone err="".
  [[nodiscard]] std::optional<Config> try_parse_config(std::string const & path, std::string * err);

}  // namespace render
