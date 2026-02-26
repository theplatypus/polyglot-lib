# Python Example (rust-core)

This client uses the compiled `mylib_rs` PyO3 extension from `crates/py`.

## Setup

```bash
python -m venv .venv
source .venv/bin/activate
pip install --upgrade pip maturin
maturin develop --manifest-path ../../crates/py/Cargo.toml
pip install -e .
```

## Run

```bash
PYTHONPATH=src python -m examples.run_demo
PYTHONPATH=src pytest -q tests
```

Optional Polars integration:

```bash
PYTHONPATH=src python -m examples.polars_demo
```
