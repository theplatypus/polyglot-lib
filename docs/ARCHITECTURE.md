# Architecture Overview

This repository compares two implementation strategies while preserving one API contract and nearly identical client code.

## Main branch (`main`)

- Contains API definition, docs, client demos, CI scaffolding.
- Uses mock backends only.
- Objective: stabilize interface and integration expectations first.

## Branch A (`rust-core`)

Planned structure:

```text
/Cargo.toml
/crates/core      # pure Rust compute kernels
/crates/c_abi     # C ABI facade for C++ client
/crates/py        # PyO3 bindings (NumPy interop)
/crates/wasm      # wasm-bindgen bindings
```

Flow:

- C++ client -> C ABI crate -> Rust core
- Python binding -> Rust core directly
- WASM binding -> Rust core directly

## Branch B (`cpp-core`)

Planned structure:

```text
/cpp/include      # public C++ API
/cpp/src          # core implementation
/cpp/pybind       # pybind11 module
/cpp/wasm         # Emscripten glue/build
/cpp/CMakeLists.txt
```

Flow:

- C++ client -> native C++ core
- Python binding -> pybind11 -> C++ core
- WASM binding -> Emscripten output -> C++ core

## Comparison dimensions

- Build system complexity and ergonomics
- ABI stability and FFI safety surface
- Binding maintenance cost
- Runtime performance and package/bundle sizes
- CI complexity and execution time
