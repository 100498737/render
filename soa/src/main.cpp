#include <cstdio>  // para usar std::println(stderr, ...)
#include <print>
#include <string>

#include "render/config.hpp"  // <- TA2: parser de config
#include "vector.hpp"         // <- igual que en la plantilla

namespace {

  [[nodiscard]] int handle_bad_argc(int provided_args) {
    // EXACTO como en el enunciado; mejor a stderr
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  // argv[1] = config, argv[2] = scene, argv[3] = salida
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  std::string const config_path = argv[1];
  std::string const scene_path  = argv[2];
  std::string const out_path    = argv[3];
  (void) scene_path;
  (void) out_path;  // TODO: se usarán en TA3/TD2

  // TA2: cargar configuración (parser real)
  std::string err;
  auto cfg = render::try_parse_config(config_path, &err);
  if (!cfg) {
    std::println(stderr, "Error: {}", err);
    return 1;
  }

  // --- CÓDIGO BASE DE LA PLANTILLA (lo mantenemos) ---
  std::println("Starting SOA rendering");
  render::vector vec{1.0, 2.0, 3.0};
  std::println("Vector magnitude: {}", vec.magnitude());

  // TODO (futuro):
  // - cargar escena (TA3)
  // - construir cámara (TB2) con parámetros de cfg
  // - crear buffer AOS/SOA (TD1)
  // - renderizar (TC1/TC2)
  // - volcar PPM (TD2) con gamma cfg->gamma

  return 0;
}
