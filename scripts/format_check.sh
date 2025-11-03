#!/usr/bin/env bash
set -Eeuo pipefail
clang-format -n --Werror \
  common/include/render/*.hpp  common/src/*.cpp \
  aos/include/render/*.hpp     aos/src/*.cpp \
  soa/include/render/*.hpp     soa/src/*.cpp
echo "OK: formato correcto"
