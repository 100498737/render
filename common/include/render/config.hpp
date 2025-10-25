#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "render/scene.hpp"  // necesitamos Scene completa para std::optional<Scene>
#include "render/vector.hpp"

namespace render {

  // Configuración de la cámara/render que pide el enunciado
  struct Config {
    std::uint32_t width{400};
    std::uint32_t height{225};

    // FOV vertical en grados
    double vertical_fov_deg{60.0};

    // Parámetros de cámara
    Vec3 lookfrom{
      Vec3{0.0, 0.0, 1.0}
    };
    Vec3 lookat{
      Vec3{0.0, 0.0, 0.0}
    };
    Vec3 vup{
      Vec3{0.0, 1.0, 0.0}
    };

    // Muestreo y RNG
    std::uint32_t samples_per_pixel{4};
    std::uint64_t seed{42ULL};
  };

  // Parseo de configuración (TA2)
  [[nodiscard]] std::optional<Config> try_parse_config(std::string const & filename,
                                                       std::string * err);

  // Parseo de escena (TA2)
  [[nodiscard]] std::optional<Scene> try_parse_scene(std::string const & filename,
                                                     std::string * err);

}  // namespace render
