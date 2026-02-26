# WASM Notes

These notes apply to both implementation branches.

## Data interchange

- Scalars: direct number arguments.
- Vectors: `Float64Array` for input/output.
- Matrices: `Float64Array` + explicit `[rows, cols]`.
- Geometry points: flattened `Float64Array` (`[x0, y0, x1, y1, ...]`) or structured JS arrays depending on binding ergonomics.

## Shape checks

Bindings must validate:

- vector length equality
- matrix rank and inner-dimension compatibility
- output buffer capacity for into variants
- minimum polygon uniqueness constraints

## Performance considerations

- Minimize copies between JS and WASM memory.
- Reuse output buffers for into variants in hot paths.
- Avoid allocating temporary JS arrays for large tensors when possible.

## Error propagation

Surface meaningful JS errors with canonical codes/messages so frontend diagnostics remain comparable across branches.

## Testing

- wasm-side smoke tests for scalar/vector/tensor/geometry behavior
- at least one explicit failure-path test per domain
