declare module "../../../cpp/wasm/build/mylib_wasm_glue.js" {
  export default function init(): Promise<unknown>;
  export function add(a: number, b: number): number;
  export function div(a: number, b: number): number;
  export function axpy(a: number, x: Float64Array, y: Float64Array): Float64Array;
  export function axpy_into(a: number, x: Float64Array, y: Float64Array, out: Float64Array): void;
  export function dot(x: Float64Array, y: Float64Array): number;
  export function matmul(
    aData: Float64Array,
    aRows: number,
    aCols: number,
    bData: Float64Array,
    bRows: number,
    bCols: number
  ): Float64Array;
  export function matmul_into(
    aData: Float64Array,
    aRows: number,
    aCols: number,
    bData: Float64Array,
    bRows: number,
    bCols: number,
    out: Float64Array,
    outRows: number,
    outCols: number
  ): void;
  export function sum_axis0(xData: Float64Array, rows: number, cols: number): Float64Array;
  export function bounding_box(pointsFlat: Float64Array): Float64Array;
  export function convex_hull(pointsFlat: Float64Array): Float64Array;
  export function convex_hull_into(pointsFlat: Float64Array, outFlat: Float64Array): number;
  export function polygon_contains(
    polygonFlat: Float64Array,
    pointX: number,
    pointY: number
  ): boolean;
}
