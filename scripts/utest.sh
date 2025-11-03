#!/usr/bin/env bash
set -Eeuo pipefail

# Ejecuta los 3 ejecutables de tests vía CTest en tu build por defecto
ctest --test-dir out/build/default --build-config Release --output-on-failure
