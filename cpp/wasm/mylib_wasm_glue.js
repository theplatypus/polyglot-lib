import createModule from "./mylib_wasm_core.js";

let moduleInstance = null;

const ensure = () => {
  if (!moduleInstance) {
    throw new Error("WASM module not initialized");
  }
  return moduleInstance;
};

const readCString = (ptr) => {
  const m = ensure();
  return m.UTF8ToString(ptr);
};

const checkStatus = (status) => {
  if (status !== 0) {
    const m = ensure();
    const msg = readCString(m._mylib_last_error_message());
    throw new Error(msg || "mylib wasm error");
  }
};

const writeF64Array = (ptr, arr) => {
  const m = ensure();
  for (let i = 0; i < arr.length; i += 1) {
    m.setValue(ptr + i * 8, arr[i], "double");
  }
};

const readF64Array = (ptr, len) => {
  const m = ensure();
  const out = new Float64Array(len);
  for (let i = 0; i < len; i += 1) {
    out[i] = m.getValue(ptr + i * 8, "double");
  }
  return out;
};

const allocF64 = (arr) => {
  const m = ensure();
  const ptr = m._malloc(arr.length * 8);
  writeF64Array(ptr, arr);
  return ptr;
};

const allocPoints = (pointsFlat) => allocF64(pointsFlat);

export default async function init(options = {}) {
  if (!moduleInstance) {
    moduleInstance = await createModule(options);
  }
  return moduleInstance;
}

export function add(a, b) {
  const m = ensure();
  const out = m._malloc(8);
  checkStatus(m._mylib_add(a, b, out));
  const v = m.getValue(out, "double");
  m._free(out);
  return v;
}

export function div(a, b) {
  const m = ensure();
  const out = m._malloc(8);
  checkStatus(m._mylib_div(a, b, out));
  const v = m.getValue(out, "double");
  m._free(out);
  return v;
}

export function axpy(a, x, y) {
  const out = new Float64Array(x.length);
  axpy_into(a, x, y, out);
  return out;
}

export function axpy_into(a, x, y, out) {
  const m = ensure();
  const xp = allocF64(x);
  const yp = allocF64(y);
  const op = m._malloc(out.length * 8);
  checkStatus(m._mylib_axpy(a, xp, x.length, yp, y.length, op, out.length));
  out.set(readF64Array(op, out.length));
  m._free(xp);
  m._free(yp);
  m._free(op);
}

export function dot(x, y) {
  const m = ensure();
  const xp = allocF64(x);
  const yp = allocF64(y);
  const out = m._malloc(8);
  checkStatus(m._mylib_dot(xp, x.length, yp, y.length, out));
  const v = m.getValue(out, "double");
  m._free(xp);
  m._free(yp);
  m._free(out);
  return v;
}

export function matmul(aData, aRows, aCols, bData, bRows, bCols) {
  const out = new Float64Array(aRows * bCols);
  matmul_into(aData, aRows, aCols, bData, bRows, bCols, out, aRows, bCols);
  return out;
}

export function matmul_into(aData, aRows, aCols, bData, bRows, bCols, out, outRows, outCols) {
  const m = ensure();
  const ap = allocF64(aData);
  const bp = allocF64(bData);
  const op = m._malloc(out.length * 8);
  checkStatus(m._mylib_matmul(ap, aRows, aCols, bp, bRows, bCols, op, outRows, outCols));
  out.set(readF64Array(op, out.length));
  m._free(ap);
  m._free(bp);
  m._free(op);
}

export function sum_axis0(xData, rows, cols) {
  const m = ensure();
  const xp = allocF64(xData);
  const op = m._malloc(cols * 8);
  checkStatus(m._mylib_sum_axis0(xp, rows, cols, op, cols));
  const out = readF64Array(op, cols);
  m._free(xp);
  m._free(op);
  return out;
}

export function bounding_box(pointsFlat) {
  const m = ensure();
  const pp = allocPoints(pointsFlat);
  const xmin = m._malloc(8);
  const ymin = m._malloc(8);
  const xmax = m._malloc(8);
  const ymax = m._malloc(8);
  checkStatus(m._mylib_bounding_box(pp, pointsFlat.length / 2, xmin, ymin, xmax, ymax));
  const out = new Float64Array([
    m.getValue(xmin, "double"),
    m.getValue(ymin, "double"),
    m.getValue(xmax, "double"),
    m.getValue(ymax, "double")
  ]);
  [pp, xmin, ymin, xmax, ymax].forEach((p) => m._free(p));
  return out;
}

export function convex_hull(pointsFlat) {
  const m = ensure();
  const pp = allocPoints(pointsFlat);
  const maxPts = pointsFlat.length / 2;
  const out = m._malloc(pointsFlat.length * 8);
  const outN = m._malloc(8);
  checkStatus(m._mylib_convex_hull(pp, maxPts, out, maxPts, outN));
  const count = Number(m.getValue(outN, "i32"));
  const flat = readF64Array(out, count * 2);
  [pp, out, outN].forEach((p) => m._free(p));
  return flat;
}

export function convex_hull_into(pointsFlat, outFlat) {
  const m = ensure();
  const pp = allocPoints(pointsFlat);
  const op = m._malloc(outFlat.length * 8);
  const outN = m._malloc(8);
  checkStatus(m._mylib_convex_hull(pp, pointsFlat.length / 2, op, outFlat.length / 2, outN));
  const count = Number(m.getValue(outN, "i32"));
  outFlat.set(readF64Array(op, count * 2), 0);
  [pp, op, outN].forEach((p) => m._free(p));
  return count;
}

export function polygon_contains(polygonFlat, pointX, pointY) {
  const m = ensure();
  const pp = allocPoints(polygonFlat);
  const out = m._malloc(1);
  checkStatus(m._mylib_wasm_polygon_contains(pp, polygonFlat.length / 2, pointX, pointY, out));
  const v = m.getValue(out, "i8") !== 0;
  m._free(pp);
  m._free(out);
  return v;
}
