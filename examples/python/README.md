# Python Example

NumPy-first client example for the shared API contract.

- On `main`, uses pure-Python mock backend.
- On implementation branches, `MyLibClient` is expected to switch to compiled extension bindings with same methods.

## Setup

```bash
python -m venv .venv
source .venv/bin/activate
pip install -e .[dev]
```

## Run

```bash
python -m examples.run_demo
pytest
```

Optional Polars integration:

```bash
python -m examples.polars_demo
```
