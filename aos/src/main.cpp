#include <cstdio>  // std::fprintf
#include <print>   // std::println
#include <string>

namespace {

  [[nodiscard]] int handle_bad_argc(int provided_args) {
    // Mensaje EXACTO al enunciado:
    // Error: Invalid number of arguments: <n>
    std::fprintf(stderr, "Error: Invalid number of arguments: %d\n", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  // se esperan exactamente 3 argumentos del usuario:
  //   argv[1] -> config
  //   argv[2] -> scene
  //   argv[3] -> salida imagen
  if (argc != 4) {
    return handle_bad_argc(argc - 1);
  }

  // caso correcto: 3 argumentos
  std::string const config_path = argv[1];
  std::string const scene_path  = argv[2];
  std::string const out_path    = argv[3];

  // aún no hacemos parsing real ni render, eso es TA2/TA3/TD2.
  // requisito TA1: si argc==4, exit code 0 y (normalmente) sin error en stderr.
  // no imprimimos nada obligatorio al stdout en el enunciado base.
  (void) config_path;
  (void) scene_path;
  (void) out_path;

  return 0;
}
