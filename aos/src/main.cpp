#include <cstdint>
#include <cstdio>  // stderr
#include <print>   // std::println

// Estos includes ya estaban y compilaban, aunque no los usamos del todo aún.
// Déjalos porque eran parte del proyecto y no daban fallo.
#include "render/camera.hpp"
#include "render/config.hpp"
#include "render/parser.hpp"
#include "render/vector.hpp"

namespace {

  // Mensaje EXACTO que comprueba ftest.sh
  [[nodiscard]] int handle_bad_argc(int provided_args) {
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  // Queremos exactamente 3 argumentos "del usuario":
  //   argv[1] = config file path
  //   argv[2] = scene file path
  //   argv[3] = output image path
  //
  // Eso significa que argc debe ser 4 (programa + 3 args).
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  // En TA1 NO parseábamos ni abríamos los ficheros todavía.
  // Simplemente aceptábamos que "parecen correctos" y devolvíamos éxito.

  (void) argv;  // silenciar warning de variable sin usar

  return 0;
}
