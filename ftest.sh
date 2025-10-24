#!/usr/bin/env bash
set -euo pipefail
BIN_SOA=${1:-./render-soa}
BIN_AOS=${2:-./render-aos}

check() {
  local bin="$1"; local want="$2"; shift 2
  set +e
  out="$($bin "$@" 2>&1)"; code=$?
  set -e
  if [[ "$out" != "$want" || $code -ne 1 ]]; then
    echo "FAIL: $bin $*"
    echo " got:  [$out] (code $code)"
    echo " want: [$want] (code 1)"
    exit 1
  fi
}

check "$BIN_SOA" "Error: Invalid number of arguments: 0"
check "$BIN_SOA" "Error: Invalid number of arguments: 1" cfg
check "$BIN_SOA" "Error: Invalid number of arguments: 2" cfg scn
check "$BIN_SOA" "Error: Invalid number of arguments: 4" cfg scn out extra

check "$BIN_AOS" "Error: Invalid number of arguments: 0"
check "$BIN_AOS" "Error: Invalid number of arguments: 1" cfg
check "$BIN_AOS" "Error: Invalid number of arguments: 2" cfg scn
check "$BIN_AOS" "Error: Invalid number of arguments: 4" cfg scn out extra

echo "OK: CLI argument validation passed."
