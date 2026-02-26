# polyglot_lib (`cpp-core` branch)

This branch implements the shared API using C++ as the core:

- `cpp/include`, `cpp/src`: core compute + C ABI implementation
- `cpp/pybind`: Python extension via pybind11 (`mylib_cpp`)
- `cpp/wasm`: Emscripten build and JS glue

Client examples under `examples/python`, `examples/cpp`, and `examples/web` keep the same call patterns as `main` while using real bindings.

## Branch comparison model

- `main`: client + docs + API contract with mocks only
- `rust-core`: Rust core + C ABI + PyO3 + wasm-bindgen
- `cpp-core`: C++ core + pybind11 + Emscripten (this branch)

## Repository layout

```text
/cpp
  /include
  /src
  /pybind
  /wasm
  CMakeLists.txt
/api/API.md
/docs/*.md
/include/mylib.h
/examples/python
/examples/cpp
/examples/web
```

## 1) Build core and tests

```bash
cmake -S cpp -B cpp/build -DMYLIB_BUILD_PYTHON=OFF
cmake --build cpp/build
ctest --test-dir cpp/build --output-on-failure
```

## 2) Build Python extension (pybind11)

```bash
cmake -S cpp -B cpp/build -DMYLIB_BUILD_PYTHON=ON
cmake --build cpp/build --target mylib_cpp
```

Run Python demo/tests:

```bash
PYTHONPATH=cpp/build/python:examples/python/src python -m examples.run_demo
PYTHONPATH=cpp/build/python:examples/python/src pytest -q examples/python/tests
```

## 3) Build wasm with Emscripten and run web example

```bash
./cpp/wasm/build.sh
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

## 4) Build C++ example against C ABI library

Build C ABI in `cpp/build` first:

```bash
cmake -S cpp -B cpp/build -DMYLIB_BUILD_PYTHON=OFF
cmake --build cpp/build --target mylib_c_api
```

Then build/run client example:

```bash
cmake -S examples/cpp -B examples/cpp/build \
  -DMYLIB_USE_MOCK=OFF \
  -DMYLIB_LIB=$(pwd)/cpp/build/libmylib_c_api.a \
  -DMYLIB_CORE_LIB=$(pwd)/cpp/build/libmylib_core.a
cmake --build examples/cpp/build
./examples/cpp/build/mylib_cpp_example
```

## CI

`ci-cpp-core` runs:

- CMake configure/build/tests
- Python extension build + pytest
- Emscripten compile step
