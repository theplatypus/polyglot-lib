# C++ Example

This example consumes the C ABI declared in `include/mylib.h`.

- On `main`, it links a local mock backend (`src/mock_mylib.c`).
- On `rust-core` or `cpp-core`, disable mock mode and link the produced real library.

## Build (main mock mode)

```bash
cmake -S . -B build
cmake --build build
./build/mylib_cpp_example
```

## Build (real backend)

```bash
cmake -S . -B build -DMYLIB_USE_MOCK=OFF -DMYLIB_LIB=/path/to/libmylib.so
cmake --build build
./build/mylib_cpp_example
```
