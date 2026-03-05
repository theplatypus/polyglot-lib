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

## Install from GitHub Releases (no build)

Use the branch Release page and download prebuilt artifacts for your platform.

### Python wheel

1. Open the Release for your version tag (for example, `v0.1.0`).
2. Download the wheel that matches your OS, architecture, and Python version.
3. Install it directly:

```bash
python -m pip install /path/to/downloaded/mylib_cpp-...whl
```

If you want to run the Python example client with the installed wheel:

```bash
python -m pip install -e "examples/python[dev]"
python -m examples.run_demo
```

### WASM package

1. Download `mylib-<version>-wasm-pkg.zip` from the Release assets.
2. Unzip it to a local directory.
3. Copy the artifacts into `cpp/wasm/build` and run the web example:

```bash
unzip /path/to/mylib-<version>-wasm-pkg.zip -d /tmp/mylib-wasm
mkdir -p cpp/wasm/build
cp /tmp/mylib-wasm/* cpp/wasm/build/
cd examples/web
npm install
npm run dev
```

Quick Node REPL smoke test from the unzipped folder:

```bash
node
```

```js
const { add } = await import("./pkg/mylib_wasm.js");
add(2, 3);
```

### C/C++ native library + headers

1. Download:
   - `mylib-<version>-headers.zip`
   - `mylib-<version>-<os>-<arch>-native.zip`
2. Extract both archives into a local folder.
3. Link your CMake target against extracted libraries and headers:

```cmake
set(MYLIB_ROOT "/absolute/path/to/extracted/mylib")

target_include_directories(my_target PRIVATE "${MYLIB_ROOT}/include")
target_include_directories(my_target PRIVATE "${MYLIB_ROOT}/cpp/include")
target_link_libraries(my_target PRIVATE "${MYLIB_ROOT}/libmylib_c_api.a") # Linux/macOS static example
# Windows static import lib example: mylib_c_api.lib
```

## 1) Build core and tests

```bash
cmake -S cpp -B cpp/build -DMYLIB_BUILD_PYTHON=OFF
cmake --build cpp/build
ctest --test-dir cpp/build --output-on-failure
```

## 2) Build Python extension (pybind11)

```bash
python -m pip wheel ./cpp/pybind -w ./dist/wheels
python -m pip install --no-index --find-links ./dist/wheels mylib-cpp
python -m pip install -e "examples/python[dev]"
```

Run Python demo/tests:

```bash
python -m examples.run_demo
pytest -q examples/python/tests
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
  -DMYLIB_INCLUDE_DIR=$(pwd)/include \
  -DMYLIB_LIB=$(pwd)/cpp/build/libmylib_c_api.a \
  -DMYLIB_CORE_LIB=$(pwd)/cpp/build/libmylib_core.a
cmake --build examples/cpp/build
./examples/cpp/build/mylib_cpp_example
```

For release assets, you can pass just a root directory:

```bash
cmake -S examples/cpp -B examples/cpp/build \
  -DMYLIB_USE_MOCK=OFF \
  -DMYLIB_ROOT=/path/to/extracted/mylib
cmake --build examples/cpp/build
./examples/cpp/build/mylib_cpp_example
```

## CI

`ci-cpp-core` runs:

- CMake configure/build/tests
- Python extension build + pytest
- Emscripten compile step
