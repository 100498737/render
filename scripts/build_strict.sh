#!/usr/bin/env bash
set -Eeuo pipefail

# Limpio + Release
rm -rf out/build/default
cmake --preset default
cmake --build --preset gcc-release --config Release --parallel

# Tests (Multi-config => usa -C Release):
ctest --test-dir out/build/default -C Release --output-on-failure
