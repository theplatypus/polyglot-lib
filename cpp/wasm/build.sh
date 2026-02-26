#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_DIR="${ROOT_DIR}/cpp/wasm/build"
mkdir -p "${OUT_DIR}"

em++ \
  -std=c++17 \
  -O3 \
  "${ROOT_DIR}/cpp/src/mylib_core.cpp" \
  "${ROOT_DIR}/cpp/src/mylib_c_api.cpp" \
  "${ROOT_DIR}/cpp/wasm/ffi.cpp" \
  -I"${ROOT_DIR}/cpp/include" \
  -I"${ROOT_DIR}/include" \
  -sMODULARIZE=1 \
  -sEXPORT_ES6=1 \
  -sENVIRONMENT=web \
  -sALLOW_MEMORY_GROWTH=1 \
  -sEXPORTED_FUNCTIONS='["_malloc","_free","_mylib_last_error_message","_mylib_add","_mylib_div","_mylib_axpy","_mylib_dot","_mylib_matmul","_mylib_sum_axis0","_mylib_bounding_box","_mylib_convex_hull","_mylib_wasm_polygon_contains"]' \
  -sEXPORTED_RUNTIME_METHODS='["UTF8ToString","setValue","getValue"]' \
  -o "${OUT_DIR}/mylib_wasm_core.js"

cp "${ROOT_DIR}/cpp/wasm/mylib_wasm_glue.js" "${OUT_DIR}/mylib_wasm_glue.js"
echo "WASM artifacts written to ${OUT_DIR}"
