# Web Example

TypeScript + Vite client for the shared API contract.

- On `main`, uses `src/mockBackend.ts`.
- On implementation branches, replace backend wiring with WASM package bindings while keeping API calls stable.

## Run

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
