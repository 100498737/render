#!/usr/bin/env bash
set -Eeuo pipefail

# Busca cualquier uso de NOLINT en tu código (incluye variantes BEGIN/END/NEXTLINE)
# Excluimos el árbol de build por si hay copias/artefactos.
grep -R -n -E 'NOLINT' \
  common include aos soa \
  --exclude-dir=out --exclude-dir=.git --exclude-dir=_deps && {
  echo "ERROR: Se han encontrado NOLINT en el código. Elimínalos antes de entregar."
  exit 1
}

echo "OK: no hay NOLINT en el código fuente."
