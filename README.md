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
maturin develop --manifest-path crates/py/Cargo.toml
pip install -e examples/python
```

Run python demo/tests:

```bash
PYTHONPATH=examples/python/src python -m examples.run_demo
PYTHONPATH=examples/python/src pytest -q examples/python/tests
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
  -DMYLIB_LIB=$(pwd)/target/release/libmylib.a
cmake --build examples/cpp/build
./examples/cpp/build/mylib_cpp_example
```

(Use `libmylib.so`/`dylib` if preferred.)

## CI

- `.github/workflows/ci-rust-core.yml` runs Rust checks/tests, Python binding tests, wasm build/tests, and web typecheck/lint/build.
