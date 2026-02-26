import initCppWasm, {
  add,
  axpy,
  axpy_into,
  bounding_box,
  convex_hull,
  convex_hull_into,
  div,
  dot,
  matmul,
  matmul_into,
  polygon_contains,
  sum_axis0
} from "../../../cpp/wasm/build/mylib_wasm_glue.js";

import { MyLibApi, Point2 } from "./api";

const pointsToFlat = (points: Point2[]): Float64Array => {
  const out = new Float64Array(points.length * 2);
  for (let i = 0; i < points.length; i += 1) {
    out[i * 2] = points[i].x;
    out[i * 2 + 1] = points[i].y;
  }
  return out;
};

const flatToPoints = (flat: Float64Array): Point2[] => {
  const out: Point2[] = [];
  for (let i = 0; i < flat.length; i += 2) {
    out.push({ x: flat[i], y: flat[i + 1] });
  }
  return out;
};

export class CppWasmBackend implements MyLibApi {
  add(a: number, b: number): number {
    return add(a, b);
  }

  div(a: number, b: number): number {
    return div(a, b);
  }

  axpy(a: number, x: Float64Array, y: Float64Array): Float64Array {
    return axpy(a, x, y);
  }

  axpyInto(a: number, x: Float64Array, y: Float64Array, out: Float64Array): void {
    axpy_into(a, x, y, out);
  }

  dot(x: Float64Array, y: Float64Array): number {
    return dot(x, y);
  }

  matmul(
    aData: Float64Array,
    aShape: [number, number],
    bData: Float64Array,
    bShape: [number, number]
  ): Float64Array {
    return matmul(aData, aShape[0], aShape[1], bData, bShape[0], bShape[1]);
  }

  matmulInto(
    aData: Float64Array,
    aShape: [number, number],
    bData: Float64Array,
    bShape: [number, number],
    out: Float64Array,
    outShape: [number, number]
  ): void {
    matmul_into(
      aData,
      aShape[0],
      aShape[1],
      bData,
      bShape[0],
      bShape[1],
      out,
      outShape[0],
      outShape[1]
    );
  }

  sumAxis0(xData: Float64Array, xShape: [number, number]): Float64Array {
    return sum_axis0(xData, xShape[0], xShape[1]);
  }

  boundingBox(points: Point2[]): [number, number, number, number] {
    const out = bounding_box(pointsToFlat(points));
    return [out[0], out[1], out[2], out[3]];
  }

  convexHull(points: Point2[]): Point2[] {
    return flatToPoints(convex_hull(pointsToFlat(points)));
  }

  convexHullInto(points: Point2[], out: Point2[]): number {
    const flatOut = new Float64Array(out.length * 2);
    const count = convex_hull_into(pointsToFlat(points), flatOut);
    for (let i = 0; i < count; i += 1) {
      out[i] = { x: flatOut[i * 2], y: flatOut[i * 2 + 1] };
    }
    return count;
  }

  polygonContains(polygon: Point2[], point: Point2): boolean {
    return polygon_contains(pointsToFlat(polygon), point.x, point.y);
  }
}

export const loadCppWasmBackend = async (): Promise<CppWasmBackend> => {
  await initCppWasm();
  return new CppWasmBackend();
};
