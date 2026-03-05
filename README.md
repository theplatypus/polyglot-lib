# polyglot_lib (`rust-core` branch)

This branch implements the shared API using:

- `crates/core`: pure Rust compute kernels
- `crates/c_abi`: C ABI (`libmylib`) for C++ consumers
- `crates/py`: PyO3 + NumPy bindings (`mylib_rs`)
- `crates/wasm`: wasm-bindgen bindings for web clients

The client examples under `examples/python`, `examples/cpp`, and `examples/web` keep the same API usage as on `main`, but now call the real Rust implementation.

## Branch comparison model

- `main`: client + docs + API contract with mocks only
- `rust-core`: Rust core + C ABI + PyO3 + wasm-bindgen (this branch)
- `cpp-core`: C++ core + pybind11 + Emscripten

## Repository layout

```text
/Cargo.toml
/crates/core
/crates/c_abi
/crates/py
/crates/wasm
/api/API.md
/docs/*.md
/include/mylib.h
/examples/python
/examples/cpp
/examples/web
```

## Install from GitHub Releases (no build)

Use the branch Release page and download prebuilt assets for your target OS/arch.

### Python wheel

1. Open the Release for your version tag (for example, `v0.1.0`).
2. Download the wheel asset matching your OS and Python ABI compatibility.
3. Install it directly:

```bash
python -m pip install /path/to/downloaded/mylib_rs-...whl
```

If you also want to run the Python example client:

```bash
python -m pip install -e "examples/python[dev]"
python -m examples.run_demo
```

### WASM package

1. Download `mylib-<version>-wasm-pkg.zip` from the Release assets.
2. Unzip so that `crates/wasm/pkg` exists locally.
3. Run the web example:

```bash
unzip /path/to/mylib-<version>-wasm-pkg.zip -d /tmp
rm -rf crates/wasm/pkg
cp -R /tmp/pkg crates/wasm/pkg
cd examples/web
npm install
npm run dev
```

### C/C++ native library + headers

1. Download:
   - `mylib-<version>-headers.zip`
   - `mylib-<version>-<os>-<arch>-native.zip`
2. Extract both archives into a local folder.
3. Link your CMake target against the shared/static library and include headers:

```cmake
set(MYLIB_ROOT "/absolute/path/to/extracted/mylib")

target_include_directories(my_target PRIVATE "${MYLIB_ROOT}/include")
target_link_libraries(my_target PRIVATE "${MYLIB_ROOT}/libmylib.so") # Linux example
# macOS: libmylib.dylib
# Windows: mylib.lib (with mylib.dll beside executable)
```

## 1) Rust build, lint, tests

```bash
cargo fmt --check
cargo clippy --workspace --all-targets -- -D warnings
cargo test --workspace
```

## 2) Python bindings (PyO3)

Build/install extension into active venv:

```bash
python -m venv .venv
source .venv/bin/activate
pip install --upgrade pip maturin
maturin build --manifest-path crates/py/Cargo.toml --release
pip install --no-index --find-links target/wheels mylib-rs
pip install -e "examples/python[dev]"
```

Run python demo/tests:

```bash
python -m examples.run_demo
pytest -q examples/python/tests
```

Wheel build option:

```bash
maturin build --manifest-path crates/py/Cargo.toml --release
```

## 3) WASM bindings

Build wasm package for bundlers:

```bash
wasm-pack build crates/wasm --target bundler --out-dir pkg
```

Run wasm tests (headless node):

```bash
wasm-pack test --node crates/wasm
```

Run web example:

```bash
cd examples/web
npm install
npm run dev
```

The web example imports generated bindings from `crates/wasm/pkg`.

## 4) C ABI + C++ example

Build C ABI library:

```bash
cargo build -p mylib-c-abi --release
```

Then build C++ example linked against real `libmylib`:

```bash
cmake -S examples/cpp -B examples/cpp/build \
  -DMYLIB_USE_MOCK=OFF \
  -DMYLIB_INCLUDE_DIR=$(pwd)/include \
  -DMYLIB_LIB=$(pwd)/target/release/libmylib.a
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

- `.github/workflows/ci-rust-core.yml` runs Rust checks/tests, Python binding tests, wasm build/tests, and web typecheck/lint/build.
