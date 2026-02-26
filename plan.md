You are an expert polyglot library engineer (Rust/C++/Python/WASM). Create an example scaffolding repository that demonstrates how to build a portable scientific/geometry compute library usable from C++, Python, and Web (JS/WASM), following best practices in organization, documentation, testing, and CI.

You will implement two alternative cores on separate branches so they can be compared easily:
	•	Branch rust-core: Rust core + C ABI (for C++) + PyO3 (Python) + wasm-bindgen (WASM)
	•	Branch cpp-core: C++ core + pybind11 (Python) + Emscripten (WASM)

Work will be delivered in three steps exactly as follows:

⸻

STEP 1 — Main branch: client code only (no core implementation yet)

Goal

On the main branch, create the repository skeleton, documentation, CI scaffolding, and client code for C++ / Python / JS(WASM) that is ready to consume a library API, but uses stubs/mocks until core implementations are provided on the other branches.

The key is that client code stays identical across branches as much as possible, so only the core/bindings differ.

Requirements
	1.	Provide a top-level README.md that explains:
	•	what the repo demonstrates
	•	the two branch architectures and how to compare them
	•	how to run each client example once the branch core is checked out
	2.	Provide examples:
	•	examples/python/ using NumPy (and show optional integration with polars or Arrow if you can do it cleanly)
	•	examples/cpp/ using CMake
	•	examples/web/ using TypeScript + Vite
	3.	Provide a uniform API contract in a language-neutral spec:
	•	api/ folder containing an API.md describing functions, signatures, shapes, error behavior
	•	and a C header stub include/mylib.h that represents the C ABI surface (even if cpp-core won’t strictly need it, keep it for comparison)
	4.	Clients should call:
	•	scalar functions
	•	vector functions
	•	tensor (nd-array) functions
	•	geometry functions
	5.	Clients should demonstrate error handling and shape checks (e.g., mismatched vector length, invalid polygon, etc.)
	6.	Add GitHub Actions CI on main that runs:
	•	lint/format checks for Python/JS where applicable
	•	builds client projects (even if linking fails without core, you can structure CI to only validate formatting + install + typecheck on main; full build will be on branches)

On main, do NOT implement the actual core. Provide mock outputs where needed, but keep the API stable.

⸻

STEP 2 — Branch rust-core: implement Rust core + bindings

Goal

On branch rust-core, implement the full working library using:
	•	crates/core in Rust (pure compute)
	•	crates/c_abi exporting stable C ABI for C++
	•	crates/py using PyO3 + numpy interop
	•	crates/wasm using wasm-bindgen (and wasm-bindgen-test)

The Python and WASM bindings should call into the Rust core crate directly (not via the C ABI). C++ calls the C ABI.

Update the README with full setup/build instructions for this branch, including:
	•	cargo build/test
	•	maturin develop or wheel build
	•	wasm-pack build (or equivalent)
	•	building the C++ example against the produced library
	•	running Python and web examples

Add CI on this branch that runs:
	•	cargo fmt --check
	•	cargo clippy -- -D warnings
	•	cargo test
	•	python tests
	•	wasm build + wasm tests (headless)

⸻

STEP 3 — Branch cpp-core: implement C++ core + bindings

Goal

On branch cpp-core, implement the full working library using:
	•	C++ core library (header + source) built with CMake
	•	Python bindings via pybind11
	•	WASM build via Emscripten (provide JS/TS glue)
	•	(Optional) also provide a C ABI layer if you want parity, but the required minimum is native C++ + bindings.

Update README with full setup/build instructions for this branch, including:
	•	building native core and C++ example
	•	building Python extension (pybind11)
	•	building WASM with emcc + running the web example

Add CI on this branch that runs:
	•	CMake configure + build + tests
	•	Python tests
	•	Emscripten build (at least compile step)

⸻

Shared API to implement on both cores

Implement these functions (same semantics across bindings). Use f64/double everywhere for simplicity.

Scalars
	•	add(a, b) -> a+b
	•	div(a, b) -> a/b with division-by-zero error

Vectors (1D arrays)
	•	axpy(a, x, y) -> a*x + y (BLAS AXPY)
	•	input: a: f64, x: [f64], y: [f64] same length
	•	output: [f64]
	•	also provide into variant: axpy_into(a, x, y, out)
	•	dot(x, y) -> f64 (length must match, non-empty)

Tensors (nd-arrays)

Pick a simple tensor representation that maps cleanly:
	•	Python: NumPy arrays
	•	WASM: TypedArrays + explicit shape
	•	C++: pointer + shape

Functions:
	•	matmul(a, b) -> c for 2D matrices
	•	support only contiguous row-major for the example
	•	include shape checks
	•	provide into variant: matmul_into(a, b, out)
	•	sum_axis0(x) -> y sum over axis 0 for a 2D tensor
	•	provide into variant if reasonable

Geometry

Use 2D geometry in lon/lat-like plane (don’t do geodesic here; keep planar).

Represent points as (x,y) f64 pairs.

Functions:
	•	bounding_box(points) -> (xmin, ymin, xmax, ymax) with error if empty
	•	convex_hull(points) -> hull_points
	•	implement a known algorithm (e.g., Andrew monotonic chain)
	•	define behavior for duplicates and collinear points
	•	polygon_contains(polygon, point) -> bool
	•	polygon is a closed ring (but allow caller to omit last==first; handle it)
	•	error if polygon has < 3 unique points

For geometry functions:
	•	Provide pure functions returning new vectors
	•	For at least one (e.g., convex hull), provide an into variant:
	•	convex_hull_into(points, out_points) -> n_out (write into caller buffer and return count, or error if buffer too small)

⸻

Error handling rules (must be consistent)
	•	Never panic/throw across FFI boundaries.
	•	Define a small set of error codes/messages:
	•	OK
	•	INVALID_ARGUMENT
	•	SHAPE_MISMATCH
	•	DIV_BY_ZERO
	•	BUFFER_TOO_SMALL
	•	INTERNAL_ERROR
	•	For C ABI (and where relevant in C++/WASM), include a last_error_message() accessor for debugging.
	•	Python:
	•	map errors to Python exceptions (ValueError, ZeroDivisionError, etc.)
	•	WASM:
	•	throw JS exceptions (via Result<T, JsValue> in Rust or manual throw in C++ glue)

⸻

Best practices to follow (must)
	•	Clear workspace layout, clean module boundaries, minimal dependencies
	•	Documentation:
	•	Rust doc comments / C++ Doxygen-style comments
	•	docs/ folder:
	•	ARCHITECTURE.md describing both branch approaches
	•	FFI_CONTRACT.md describing array layout, ownership, safety, error mapping
	•	WASM_NOTES.md describing TypedArray interop, shapes, and performance considerations
	•	Testing:
	•	unit tests for core logic
	•	binding-level smoke tests
	•	Python pytest tests that compare results with NumPy reference for linear algebra
	•	optional property tests (nice-to-have, not required)
	•	Formatting/linting:
	•	Rust fmt + clippy
	•	C++ format recommendations (clang-format config is a plus)
	•	Python lint (ruff) optional but welcome
	•	TS formatting (prettier) optional but welcome
	•	CI:
	•	separate workflows or conditional steps per branch if needed
	•	Keep everything buildable on Linux. Provide notes for macOS/Windows but Linux is the target.

⸻

Repository layout guidance

You may adjust slightly, but keep it coherent:

/README.md
/api/API.md
/docs/*.md
/include/mylib.h
/examples/
  /python/
  /cpp/
  /web/
/.github/workflows/...

On rust-core branch add:

/Cargo.toml (workspace)
 /crates/core
 /crates/c_abi
 /crates/py
 /crates/wasm

On cpp-core branch add:

/cpp/
  /include
  /src
  /pybind
  /wasm
  CMakeLists.txt


⸻

Proceed now, implementing STEP 1 (main branch) first, but also include at the end a short checklist of what you will add/modify in STEP 2 and STEP 3 to complete the other branches.
