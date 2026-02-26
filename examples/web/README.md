# Web Example (rust-core)

This client uses wasm-bindgen output from `crates/wasm/pkg`.

## Build wasm package

```bash
wasm-pack build ../../crates/wasm --target bundler --out-dir pkg
```

## Run web app

```bash
npm install
npm run dev
```

## Validate

```bash
npm run lint
npm run typecheck
npm run build
```
