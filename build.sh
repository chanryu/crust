#! /usr/bin/env bash
set -euo pipefail

# Generate sumtype headers for each tests/*.enum.json
# Outputs go to tests/<basename>.enum.hpp (kept out of VCS via .gitignore)
OUT_DIR="tests"
mkdir -p "$OUT_DIR"

shopt -s nullglob
specs=(tests/*.enum.json)
if [ ${#specs[@]} -eq 0 ]; then
  echo "No .enum.json specs found in tests/"
else
  echo "Generating sumtype headers into $OUT_DIR"
  for spec in "${specs[@]}"; do
    base=$(basename "$spec" .enum.json)
    out="$OUT_DIR/${base}.enum.hpp"
    echo "  -> $spec -> $out"
    python3 scripts/gen_sumtype.py "$spec" "$out"
  done
fi

# Run the normal CMake configure/build
cmake --preset=default
cmake --build --preset=default
