# C++ Example (cpp-core)

This client consumes the C ABI declared in `include/mylib.h`.

## Build C++ core C ABI

```bash
cmake -S ../../cpp -B ../../cpp/build -DMYLIB_BUILD_PYTHON=OFF
cmake --build ../../cpp/build --target mylib_c_api
```

## Build example against C ABI

```bash
cmake -S . -B build \
  -DMYLIB_USE_MOCK=OFF \
  -DMYLIB_LIB=$PWD/../../cpp/build/libmylib_c_api.a \
  -DMYLIB_CORE_LIB=$PWD/../../cpp/build/libmylib_core.a
cmake --build build
./build/mylib_cpp_example
```

Mock mode remains available:

```bash
cmake -S . -B build -DMYLIB_USE_MOCK=ON
cmake --build build
./build/mylib_cpp_example
```
