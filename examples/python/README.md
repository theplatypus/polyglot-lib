# Python Example (rust-core)

This client expects `mylib_rs` to be installed as a wheel (release wheel or locally built wheel).

## Option 1: Install from GitHub Release wheel

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install /path/to/mylib_rs-...whl
python -m pip install -e ".[dev]"
```

## Option 2: Build wheel from source, then install

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip maturin
maturin build --manifest-path ../../crates/py/Cargo.toml --release
python -m pip install --no-index --find-links ../../target/wheels mylib-rs
python -m pip install -e ".[dev]"
```

## Run

```bash
python -m examples.run_demo
pytest -q tests
```

Optional Polars integration:

```bash
python -m examples.polars_demo
```
