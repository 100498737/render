#!/usr/bin/env bash
set -euo pipefail

echo "=== Functional tests TA1 ==="

# Ruta del ejecutable de render-soa que genera CMake con el preset gcc-release
BIN_SOA="out/build/default/soa/Release/render-soa"

run_case() {
  local args_str="$1"
  local want_rc="$2"
  local want_err="$3"

  # Ejecuta, capturando stdout/stderr y el RC inmediatamente
  # shellcheck disable=SC2086
  $BIN_SOA $args_str >stdout.txt 2>stderr.txt
  local rc=$?

  local got_out=""
  local got_err=""
  [[ -f stdout.txt ]] && got_out="$(cat stdout.txt || true)"
  [[ -f stderr.txt ]] && got_err="$(cat stderr.txt || true)"

  if [[ "$rc" -ne "$want_rc" ]]; then
    echo "ASSERT FAIL: RC ($args_str)"
    echo " got : [$rc]"
    echo " want: [$want_rc]"
    exit 1
  fi

  if [[ "$want_err" != "-" && "$got_err" != "$want_err" ]]; then
    echo "ASSERT FAIL: STDERR ($args_str)"
    echo " got : [$got_err]"
    echo " want: [$want_err]"
    exit 1
  fi
}

# TA1: exactamente 3 argumentos de usuario => 0,1,2 deben fallar con RC=1 y mensaje exacto
run_case ""                      1 "Error: Invalid number of arguments: 0"
run_case "cfg.txt"               1 "Error: Invalid number of arguments: 1"
run_case "cfg.txt scn.txt"       1 "Error: Invalid number of arguments: 2"

# Caso correcto (3 args): RC=0, sin exigir mensaje de error
run_case "cfg.txt scn.txt out.ppm" 0 "-"

echo "ALL GOOD ✅"
