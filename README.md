# polyglot_lib

Scaffolding repository for comparing two implementation strategies of the same scientific/geometry compute API exposed to:

- C++ (native client)
- Python (NumPy-first client)
- Web (TypeScript + JS/WASM-style client)

This branch (`main`) intentionally contains **no real core implementation**. It defines the API contract, docs, and client examples using mock backends so integration points and error behavior are fixed before core work begins.

## Branches

- `rust-core`: Rust compute core + C ABI (for C++) + PyO3 (Python) + wasm-bindgen (Web)
- `cpp-core`: C++ compute core + pybind11 (Python) + Emscripten (Web)

The client-side examples in `examples/python`, `examples/cpp`, and `examples/web` are designed to stay as stable as possible across both branches.

## What this repo demonstrates

- Language-neutral API contract in [`api/API.md`](api/API.md)
- C ABI surface in [`include/mylib.h`](include/mylib.h)
- Shared error model across bindings
- Client examples for scalar/vector/tensor/geometry functions
- Error handling examples (division-by-zero, shape mismatch, invalid polygon, small output buffer)
- CI scaffolding that validates formatting/type/build paths on `main`

## Repository layout

```text
/api/API.md
/docs/ARCHITECTURE.md
/docs/FFI_CONTRACT.md
/docs/WASM_NOTES.md
/include/mylib.h
/examples/python
/examples/cpp
/examples/web
/.github/workflows/ci-main.yml
```

## Run examples on main (mock backends)

### Python

```bash
cd examples/python
python -m venv .venv
source .venv/bin/activate
pip install -e .[dev]
python -m examples.run_demo
pytest
```

Optional Polars integration demonstration:

```bash
python -m examples.polars_demo
```

### C++ (CMake)

```bash
cmake -S examples/cpp -B examples/cpp/build
cmake --build examples/cpp/build
./examples/cpp/build/mylib_cpp_example
```

By default on `main`, CMake builds a local mock C ABI implementation. On implementation branches, set `MYLIB_USE_MOCK=OFF` and link against real produced library artifacts.

### Web (TypeScript + Vite)

```bash
cd examples/web
npm install
npm run dev
```

Typecheck/lint/build:

```bash
npm run typecheck
npm run lint
npm run build
```

## Compare architectures once implementation branches are available

1. Checkout `rust-core` and build all targets (Rust core + C ABI + Python + wasm).
2. Run the same client examples.
3. Checkout `cpp-core` and do the same with CMake + pybind11 + Emscripten.
4. Compare:
   - code organization and binding complexity
   - build ergonomics and CI runtime
   - performance and binary/package size
   - debuggability and error mapping behavior

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for intended comparison points.
