# Python Example (cpp-core)

This client uses the compiled `mylib_cpp` pybind11 extension from `cpp/pybind`.

## Build extension

```bash
cmake -S ../../cpp -B ../../cpp/build -DMYLIB_BUILD_PYTHON=ON
cmake --build ../../cpp/build --target mylib_cpp
```

## Run

```bash
PYTHONPATH=../../cpp/build/python:src python -m examples.run_demo
PYTHONPATH=../../cpp/build/python:src pytest -q tests
```

Optional Polars integration:

```bash
PYTHONPATH=../../cpp/build/python:src python -m examples.polars_demo
```
