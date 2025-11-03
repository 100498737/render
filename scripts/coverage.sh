#!/usr/bin/env bash
set -Eeuo pipefail

BUILD_DIR="out/build/coverage"
rm -rf "$BUILD_DIR"

# 1) Configura build de cobertura (GCC + gcov)
cmake -S . -B "$BUILD_DIR" -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=g++-14 \
  -DCMAKE_CXX_FLAGS="-O0 -g --coverage" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
  -DCMAKE_SHARED_LINKER_FLAGS="--coverage"

# 2) Compila y ejecuta tests (genera .gcda)
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "== Resumen (si hay gcovr) =="
if command -v gcovr >/dev/null 2>&1; then
  gcovr -r . \
    --filter 'common/|aos/|soa/' \
    --exclude 'out/|_deps/|third_party/' \
    --txt --print-summary || true
else
  echo "(gcovr no encontrado o no usable; continúo con gcov)"
fi

# 3) Verificación DURA: 100% por función en cada fichero (vía gcov)
TMP_REPORT="$(mktemp)"
pushd "$BUILD_DIR" >/dev/null

# Ejecuta gcov sobre cada unidad con datos de cobertura
# -pb: nombres de fichero 'bonitos' y relativos al pwd
find . -name "*.gcno" -print0 | xargs -0 -n1 gcov -pb -o . >>"$TMP_REPORT" 2>/dev/null || true

popd >/dev/null

# 4) Comprueba Functions executed == 100.00% para ficheros de (common|aos|soa)
FAILED=0
awk '
  /^File /{
    file=$0
    # Extrae la ruta después de "File "
    sub(/^File /,"",file)
    if (file ~ /(common|aos|soa)\//) in_scope=1; else in_scope=0
  }
  in_scope && /^Functions executed:/{
    # Formato típico: "Functions executed:100.00% of 12"
    perc=$3; gsub("%","",perc)
    if (perc+0 < 100.0) {
      print "FALLO cobertura funciones en:", file, "=>", $0
      failed=1
    }
  }
  END{ if (failed) exit 1 }
' "$TMP_REPORT" || FAILED=1

if [ "$FAILED" -ne 0 ]; then
  echo "ERROR: Cobertura por función < 100% en uno o más ficheros de (common|aos|soa)."
  echo "Arriba se muestra el/los ficheros y su línea 'Functions executed: ...'."
  exit 1
fi

echo "OK: Cobertura por función = 100% en todos los ficheros de (common|aos|soa)."
