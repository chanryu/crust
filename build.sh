#! /usr/bin/env bash
set -euo pipefail

# CMake will handle generation of sumtype headers from tests/*.enum.json
cmake --preset=default
cmake --build --preset=default
