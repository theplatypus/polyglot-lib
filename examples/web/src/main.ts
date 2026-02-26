import { loadWasmBackend } from "./wasmBackend";

const render = async (): Promise<void> => {
  const lib = await loadWasmBackend();
  const lines: string[] = [];

  const push = (label: string, value: unknown): void => {
    lines.push(`${label}: ${JSON.stringify(value)}`);
  };

  push("add", lib.add(2, 3));
  push("div", lib.div(9, 3));

  const x = new Float64Array([1, 2, 3]);
  const y = new Float64Array([0.5, 1.5, 2.5]);
  push("axpy", Array.from(lib.axpy(2, x, y)));

  const out = new Float64Array(3);
  lib.axpyInto(2, x, y, out);
  push("axpy_into", Array.from(out));
  push("dot", lib.dot(x, y));

  const a = new Float64Array([1, 2, 3, 4]);
  const b = new Float64Array([5, 6, 7, 8]);
  push("matmul", Array.from(lib.matmul(a, [2, 2], b, [2, 2])));
  push("sum_axis0", Array.from(lib.sumAxis0(a, [2, 2])));

  const points = [
    { x: 0, y: 0 },
    { x: 2, y: 0 },
    { x: 2, y: 2 },
    { x: 0, y: 2 },
    { x: 1, y: 1 }
  ];
  push("bounding_box", lib.boundingBox(points));
  push("convex_hull", lib.convexHull(points));

  const hullOut = new Array(16).fill({ x: 0, y: 0 });
  const hullN = lib.convexHullInto(points, hullOut);
  push("convex_hull_into_count", hullN);

  push("polygon_contains_inside", lib.polygonContains(points.slice(0, 4), { x: 1, y: 1 }));
  push("polygon_contains_outside", lib.polygonContains(points.slice(0, 4), { x: 3, y: 1 }));

  try {
    lib.div(1, 0);
  } catch (err) {
    push("expected_div_error", err instanceof Error ? err.message : String(err));
  }

  try {
    lib.axpy(1, new Float64Array([1]), new Float64Array([1, 2]));
  } catch (err) {
    push("expected_shape_error", err instanceof Error ? err.message : String(err));
  }

  const output = document.querySelector<HTMLPreElement>("#output");
  if (output) {
    output.textContent = lines.join("\n");
  }
};

void render();
