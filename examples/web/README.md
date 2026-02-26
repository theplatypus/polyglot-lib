# Web Example (cpp-core)

This client uses Emscripten output from `cpp/wasm/build`.

## Build wasm artifacts

```bash
./cpp/wasm/build.sh
```

## Run web app

```bash
cd examples/web
npm install
npm run dev
```

## Validate

```bash
npm run lint
npm run typecheck
npm run build
```
