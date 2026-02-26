# FFI Contract

This document defines cross-language memory, ownership, and safety rules.

## Array layout

- 1D vectors: contiguous `f64` buffer with explicit length.
- 2D tensors: contiguous row-major `f64` buffer with explicit `(rows, cols)`.
- No strided, non-contiguous, or Fortran-order support in this example.

## Ownership

- Caller owns all input and output buffers.
- Functions never retain pointers after return.
- Into variants write into caller-provided buffers and validate capacity.

## Errors

Status codes:

- `OK`
- `INVALID_ARGUMENT`
- `SHAPE_MISMATCH`
- `DIV_BY_ZERO`
- `BUFFER_TOO_SMALL`
- `INTERNAL_ERROR`

Rules:

- Do not throw/panic across ABI boundaries.
- On failure, output buffers are either untouched or partially written; callers should check status first.
- `last_error_message()` provides debug message for the latest error on current thread/context (implementation-dependent detail in branch docs).

## Geometry data

`Point2` is an array of `{double x; double y;}` structs. Polygon input may be open or closed.

## Python mapping

- `INVALID_ARGUMENT`, `SHAPE_MISMATCH` -> `ValueError`
- `DIV_BY_ZERO` -> `ZeroDivisionError`
- `BUFFER_TOO_SMALL`, `INTERNAL_ERROR` -> `RuntimeError`

## WASM mapping

- Rust branch: `Result<T, JsValue>` and throw JS `Error`
- C++ branch: Emscripten glue throws JS `Error`
