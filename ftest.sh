#!/usr/bin/env bash
set -euo pipefail

echo "=== Functional tests TA1 ==="

# Ruta del ejecutable de render-soa que genera CMake con el preset gcc-release
BIN_SOA="out/build/default/soa/Release/render-soa"

#######################################
# Test 1: SIN argumentos
#######################################

# ejecutamos el binario SIN args
# redirigimos stdout y stderr a ficheros temporales
"$BIN_SOA" >stdout0.txt 2>stderr0.txt || true
rc0=$?

want_rc0=1
want_err0="Error: Invalid number of arguments: 0"

got_out0="$(cat stdout0.txt || true)"
got_err0="$(cat stderr0.txt || true)"

# assert exit code
if [ "$rc0" -ne "$want_rc0" ]; then
  echo "ASSERT FAIL: RC (0 args)"
  echo " got : [$rc0]"
  echo " want: [$want_rc0]"
  exit 1
fi

# assert stderr exact
if [ "$got_err0" != "$want_err0" ]; then
  echo "ASSERT FAIL: STDERR (0 args)"
  echo " got : [$got_err0]"
  echo " want: [$want_err0]"
  exit 1
fi

# assert stdout empty
if [ -n "$got_out0" ]; then
  echo "ASSERT FAIL: STDOUT not empty (0 args)"
  echo " got : [$got_out0]"
  echo " want: []"
  exit 1
fi

#######################################
# Test 2: CON 3 argumentos
#######################################

"$BIN_SOA" cfg.txt scn.txt out.ppm >stdout1.txt 2>stderr1.txt || true
rc1=$?

want_rc1=0

got_out1="$(cat stdout1.txt || true)"
got_err1="$(cat stderr1.txt || true)"

# assert exit code
if [ "$rc1" -ne "$want_rc1" ]; then
  echo "ASSERT FAIL: RC (3 args)"
  echo " got : [$rc1]"
  echo " want: [$want_rc1]"
  exit 1
fi

# assert stderr vacío
if [ -n "$got_err1" ]; then
  echo "ASSERT FAIL: STDERR not empty (3 args)"
  echo " got : [$got_err1]"
  echo " want: []"
  exit 1
fi

#######################################
# Si hemos llegado aquí, todo OK
#######################################
echo "ALL GOOD ✅"
echo
echo "--- expected CLI transcript example ---"
echo "\$ out/build/default/soa/Release/render-soa"
echo "Error: Invalid number of arguments: 0"
echo "\$? = 1"
echo
echo "\$ out/build/default/soa/Release/render-soa cfg.txt scn.txt out.ppm"
echo
echo "\$? = 0"
