#!/usr/bin/env bash
set -euo pipefail

# Ruta al binario SOA en gcc-release
BIN_SOA="out/build/default/soa/Release/render-soa"

echo "=== Functional tests TA1 ==="

# ---------- helper para testear una llamada ----------
run_and_check() {
    local desc="$1"        # descripción humana del caso
    local expect_rc="$2"   # código de salida esperado
    local expect_err="$3"  # substring esperado en stderr (puede ser "")

    # Ejecutamos el binario, capturando stderr y rc.
    # stdout no nos importa porque debe estar vacío en ambos casos.
    set +e
    ERR_MSG="$("$BIN_SOA" "${@:4}" 2>&1 1>/dev/null)"
    RC=$?
    set -e

    # Comprobación código retorno
    if [ "$RC" -ne "$expect_rc" ]; then
        echo "ASSERT FAIL: RC ($desc)"
        echo " got : [$RC]"
        echo " want: [$expect_rc]"
        exit 1
    fi

    # Comprobación del mensaje de error (solo si esperamos uno)
    if [ -n "$expect_err" ]; then
        if [[ "$ERR_MSG" != "$expect_err" ]]; then
            echo "ASSERT FAIL: STDERR ($desc)"
            echo " got : [$ERR_MSG]"
            echo " want: [$expect_err]"
            exit 1
        fi
    else
        # No esperamos error -> stderr tiene que estar vacío
        if [ -n "$ERR_MSG" ]; then
            echo "ASSERT FAIL: STDERR ($desc)"
            echo " got non-empty stderr: [$ERR_MSG]"
            echo " want: []"
            exit 1
        fi
    fi
}

# ---------- caso 1: sin argumentos de usuario ----------
# Esperamos:
#   stderr: "Error: Invalid number of arguments: 0"
#   rc:     1
run_and_check \
    "0 args" \
    1 \
    "Error: Invalid number of arguments: 0" \
    # no args after this line, so BIN_SOA is called with 0 extra args

# ---------- caso 2: 3 argumentos válidos ----------
# Esperamos:
#   stderr: "" (vacío)
#   rc:     0
run_and_check \
    "3 args" \
    0 \
    "" \
    cfg.txt scene.txt out.ppm


echo "ALL GOOD ✅"
echo
echo "--- expected CLI transcript example ---"
echo "\$ $BIN_SOA"
echo "Error: Invalid number of arguments: 0"
echo "\$? = 1"
echo
echo "\$ $BIN_SOA cfg.txt scene.txt out.ppm"
echo
echo "\$? = 0"
