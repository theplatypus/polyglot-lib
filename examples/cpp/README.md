# C++ Example (rust-core)

This example consumes the C ABI in `include/mylib.h`.

- Build Rust C ABI library from `crates/c_abi`.
- Link this C++ client against generated `libmylib`.

## Build Rust C ABI

```bash
cargo build -p mylib-c-abi --release
```

## Build C++ example against real library

Approach A (single root for extracted release assets):

```bash
cmake -S . -B build -DMYLIB_USE_MOCK=OFF -DMYLIB_ROOT=/path/to/extracted/mylib
cmake --build build
./build/mylib_cpp_example
```

Approach B (explicit library/include paths):

```bash
cmake -S . -B build \
  -DMYLIB_USE_MOCK=OFF \
  -DMYLIB_INCLUDE_DIR=$PWD/../../include \
  -DMYLIB_LIB=$PWD/../../target/release/libmylib.a
cmake --build build
./build/mylib_cpp_example
```

Mock mode remains available for quick local checks:

```bash
cmake -S . -B build -DMYLIB_USE_MOCK=ON
cmake --build build
./build/mylib_cpp_example
```
