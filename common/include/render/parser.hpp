#pragma once

#include <optional>
#include <string>

#include "render/config.hpp"
#include "render/scene.hpp"

namespace render {

  // Parseo de configuración (TA2)
  // Devuelve std::nullopt en caso de error y rellena *err con "Error: ...".
  [[nodiscard]] std::optional<Config> try_parse_config(std::string const & filename,
                                                       std::string * err);

  // Parseo de escena (se usará plenamente en TA3, pero ya lo exponemos)
  // Mismo contrato: std::nullopt si error, mensaje en *err.
  [[nodiscard]] std::optional<Scene> try_parse_scene(std::string const & filename,
                                                     std::string * err);

}  // namespace render
