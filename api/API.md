# MyLib API Contract (Language Neutral)

Version: `v0` (scaffold contract)

All implementations (Rust core and C++ core branches) must keep these semantics aligned.

## Numeric type

- Scalar type: `f64` / `double`
- Arrays are contiguous row-major unless stated otherwise.

## Error model

Canonical status set:

- `OK`
- `INVALID_ARGUMENT`
- `SHAPE_MISMATCH`
- `DIV_BY_ZERO`
- `BUFFER_TOO_SMALL`
- `INTERNAL_ERROR`

Behavior:

- No panic/exception escapes over FFI boundaries.
- C ABI returns explicit status and exposes `last_error_message()`.
- Python maps to Python exceptions (`ValueError`, `ZeroDivisionError`, `RuntimeError`).
- WASM surfaces JS exceptions.

## Scalars

- `add(a: f64, b: f64) -> f64`
- `div(a: f64, b: f64) -> f64`
  - error: `DIV_BY_ZERO` when `b == 0.0`

## Vectors (1D)

- `axpy(a: f64, x: [f64], y: [f64]) -> [f64]`
  - computes `a*x[i] + y[i]`
  - error: `SHAPE_MISMATCH` if lengths differ
- `axpy_into(a: f64, x: [f64], y: [f64], out: mutable [f64])`
  - same constraints + output length check
- `dot(x: [f64], y: [f64]) -> f64`
  - error: `SHAPE_MISMATCH` if lengths differ
  - error: `INVALID_ARGUMENT` if empty

## Tensors (2D matrices)

Representation:

- data pointer/TypedArray/NumPy array
- explicit shape `(rows, cols)`
- contiguous row-major

Functions:

- `matmul(a: Matrix, b: Matrix) -> Matrix`
  - `(m,k) x (k,n) -> (m,n)`
  - error: `SHAPE_MISMATCH` if inner dims mismatch
- `matmul_into(a, b, out)`
  - `out` must be `(m,n)`
- `sum_axis0(x: Matrix(m,n)) -> Vector(n)`
  - sum over rows for each column

## Geometry (2D planar)

Point type:

- `Point2 { x: f64, y: f64 }`

Functions:

- `bounding_box(points: [Point2]) -> (xmin, ymin, xmax, ymax)`
  - error: `INVALID_ARGUMENT` if empty
- `convex_hull(points: [Point2]) -> [Point2]`
  - algorithm: Andrew monotonic chain
  - duplicates ignored
  - collinear boundary behavior: returns extreme points only (minimal hull)
- `convex_hull_into(points, out_points) -> n_out`
  - writes hull points into caller buffer
  - error: `BUFFER_TOO_SMALL` if capacity insufficient
- `polygon_contains(polygon: [Point2], point: Point2) -> bool`
  - polygon ring may be open or closed; implementation normalizes
  - error: `INVALID_ARGUMENT` if polygon has fewer than 3 unique points

## Determinism and precision notes

- Implementations should be deterministic for same input order.
- Floating-point comparisons in tests should use tolerance.

## ABI reference

See [`include/mylib.h`](../include/mylib.h) for C ABI signatures used by C++ client and parity checks.
