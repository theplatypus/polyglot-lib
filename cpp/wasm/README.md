# cpp/wasm

Build Emscripten artifacts:

```bash
./cpp/wasm/build.sh
```

Outputs:

- `cpp/wasm/build/mylib_wasm_core.js`
- `cpp/wasm/build/mylib_wasm_core.wasm`
- `cpp/wasm/build/mylib_wasm_glue.js`

## Node smoke test

`mylib_wasm_glue.js` is generated for web-style loading. In Node, pass `wasmBinary`
explicitly so it does not try to `fetch()` a local file path.

```bash
cat > smoke_cpp.mjs <<'EOF'
import { readFile } from "node:fs/promises";
import initCppWasm, { add } from "./cpp/wasm/build/mylib_wasm_glue.js";

const wasmBinary = await readFile(new URL("./cpp/wasm/build/mylib_wasm_core.wasm", import.meta.url));
await initCppWasm({ wasmBinary });

console.log(add(2, 3));
EOF

node smoke_cpp.mjs
```
