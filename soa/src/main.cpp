#include <cstdint>
#include <cstdio>  // stderr
#include <print>   // std::println

namespace {

  // Mensaje EXACTO según TA1
  [[nodiscard]] int handle_bad_argc(int provided_args) {
    std::println(stderr, "Error: Invalid number of arguments: {}", provided_args);
    return 1;
  }

}  // namespace

int main(int argc, char * argv[]) {
  // Queremos exactamente 3 argumentos del usuario:
  //
  //   argv[1] = config file path
  //   argv[2] = scene file path
  //   argv[3] = output image path
  //
  // Eso implica argc == 4 (programa + 3 args).
  if (argc != 4) {
    // argc-1 = nº argumentos "del usuario"
    return handle_bad_argc(argc - 1);
  }

  // TA1: no hacemos nada más, éxito directo.
  (void) argv;  // silenciar warning

  return 0;
}
