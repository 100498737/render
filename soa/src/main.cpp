#include <cstdint>
#include <cstdio>   // stderr
#include <print>    // std::println
#include <string>

#include "render/camera.hpp"
#include "render/config.hpp"
#include "render/parser.hpp"
#include "render/scene.hpp"
#include "render/vector.hpp"

namespace {
  [[nodiscard]] int handle_bad_argc(int provided_args) {
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }
}

int main(int argc, char* argv[]) {
  // TA1: exactamente 3 argumentos de usuario => argc debe ser 4
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  // TA2: validar config; si falla, imprimir el mensaje del parser a stderr y devolver 1
  {
    std::string err;
    auto cfg = render::try_parse_config(argv[1], &err); // std::optional<Config>
    if (!cfg) {
      std::println(stderr, "{}", err);
      return 1;
    }
  }

  // TA3: parsear la escena; en error, imprimir y salir con 1
  {
    std::string err_scn;
    auto scn = render::try_parse_scene(argv[2], &err_scn);
    if (!scn) {
      std::println(stderr, "{}", err_scn);
      return 1;
    }
  }

  return 0;
}
