#include <cstdint>
#include <cstdio>  // for stderr
#include <print>   // std::println
#include <string>

#include "render/camera.hpp"  // camera (will be used soon)
#include "render/config.hpp"  // Config definition
#include "render/parser.hpp"  // try_parse_config, try_parse_scene
#include "render/vector.hpp"  // vector / Vec3

namespace {

  [[nodiscard]] int handle_bad_argc(int provided_args) {
    // EXACTAMENTE el mensaje que ya comprobamos en TA1
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  // argv[1] = config file
  // argv[2] = scene  file
  // argv[3] = output image (ppm)
  // => necesitamos 3 argumentos del usuario, así que argc debe ser 4
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  std::string const config_path = argv[1];
  std::string const scene_path  = argv[2];
  std::string const out_path    = argv[3];
  (void) out_path;  // todavía no escribimos imagen en TA2

  // 1. Parsear configuración
  std::string cfg_err;
  auto cfg_opt = render::try_parse_config(config_path, &cfg_err);

  if (!cfg_opt) {
    // El parser nos ha devuelto fallo. cfg_err DEBE contener un mensaje tipo "Error: ..."
    // Lo pasamos tal cual a stderr y salimos con código 1.
    if (!cfg_err.empty()) {
      std::println(stderr, "{}", cfg_err);
    } else {
      std::println(stderr, "Error: could not parse config file '{}'", config_path);
    }
    return 1;
  }

  render::Config const cfg = *cfg_opt;

  // 2. Parsear escena
  std::string scene_err;
  auto scene_opt = render::try_parse_scene(scene_path, &scene_err);

  if (!scene_opt) {
    if (!scene_err.empty()) {
      std::println(stderr, "{}", scene_err);
    } else {
      std::println(stderr, "Error: could not parse scene file '{}'", scene_path);
    }
    return 1;
  }

  render::Scene const scene = *scene_opt;
  (void) scene;  // aún no renderizamos

  // 3. (opcional pero útil para debug / TA2)
  //    construir cámara con la config ya parseada
  render::camera cam{cfg.width,  cfg.height, cfg.vertical_fov_deg,  cfg.lookfrom,
                     cfg.lookat, cfg.vup,    cfg.samples_per_pixel, cfg.seed};

  // silenciamos warnings si todavía no usamos cam en TA2
  (void) cam;

  // ÉXITO: todo parseado correctamente
  return 0;
}
