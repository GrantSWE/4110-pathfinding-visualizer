#!/usr/bin/env bash

set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_directory="${repository_root}/dist/wasm"

if ! command -v em++ >/dev/null 2>&1; then
    echo "Error: em++ was not found on PATH." >&2
    echo "Install and activate the Emscripten SDK, then source emsdk_env.sh before rerunning." >&2
    echo "Official instructions: https://emscripten.org/docs/getting_started/downloads.html" >&2
    exit 1
fi

mkdir -p "${output_directory}"

em++ \
    "${repository_root}/src/grid.cpp" \
    "${repository_root}/src/pathfinding.cpp" \
    "${repository_root}/src/wasm_bindings.cpp" \
    -I"${repository_root}/src" \
    -std=c++17 \
    -O2 \
    --bind \
    --no-entry \
    -sWASM=1 \
    -sMODULARIZE=1 \
    -sEXPORT_NAME=createPathfindingModule \
    -sENVIRONMENT=web \
    -sALLOW_MEMORY_GROWTH=1 \
    -sASSERTIONS=1 \
    -o "${output_directory}/pathfinding.js"

echo "WebAssembly build complete:"
echo "  ${output_directory}/pathfinding.js"
echo "  ${output_directory}/pathfinding.wasm"
