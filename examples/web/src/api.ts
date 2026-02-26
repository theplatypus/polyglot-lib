export type Point2 = { x: number; y: number };

export enum ErrorCode {
  INVALID_ARGUMENT = "INVALID_ARGUMENT",
  SHAPE_MISMATCH = "SHAPE_MISMATCH",
  DIV_BY_ZERO = "DIV_BY_ZERO",
  BUFFER_TOO_SMALL = "BUFFER_TOO_SMALL",
  INTERNAL_ERROR = "INTERNAL_ERROR"
}

export class MyLibError extends Error {
  constructor(
    public readonly code: ErrorCode,
    message: string
  ) {
    super(`${code}: ${message}`);
  }
}

export interface MyLibApi {
  add(a: number, b: number): number;
  div(a: number, b: number): number;
  axpy(a: number, x: Float64Array, y: Float64Array): Float64Array;
  axpyInto(a: number, x: Float64Array, y: Float64Array, out: Float64Array): void;
  dot(x: Float64Array, y: Float64Array): number;
  matmul(
    aData: Float64Array,
    aShape: [number, number],
    bData: Float64Array,
    bShape: [number, number]
  ): Float64Array;
  matmulInto(
    aData: Float64Array,
    aShape: [number, number],
    bData: Float64Array,
    bShape: [number, number],
    out: Float64Array,
    outShape: [number, number]
  ): void;
  sumAxis0(xData: Float64Array, xShape: [number, number]): Float64Array;
  boundingBox(points: Point2[]): [number, number, number, number];
  convexHull(points: Point2[]): Point2[];
  convexHullInto(points: Point2[], out: Point2[]): number;
  polygonContains(polygon: Point2[], point: Point2): boolean;
}
