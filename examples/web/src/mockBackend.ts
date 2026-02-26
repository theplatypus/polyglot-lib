import { ErrorCode, MyLibApi, MyLibError, Point2 } from "./api";

const cross = (o: Point2, a: Point2, b: Point2): number =>
  (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);

export class MockBackend implements MyLibApi {
  add(a: number, b: number): number {
    return a + b;
  }

  div(a: number, b: number): number {
    if (b === 0) {
      throw new MyLibError(ErrorCode.DIV_BY_ZERO, "denominator must be non-zero");
    }
    return a / b;
  }

  axpy(a: number, x: Float64Array, y: Float64Array): Float64Array {
    this.checkVec(x, y);
    const out = new Float64Array(x.length);
    this.axpyInto(a, x, y, out);
    return out;
  }

  axpyInto(a: number, x: Float64Array, y: Float64Array, out: Float64Array): void {
    this.checkVec(x, y);
    if (out.length !== x.length) {
      throw new MyLibError(ErrorCode.SHAPE_MISMATCH, "out length must match x/y");
    }
    for (let i = 0; i < x.length; i += 1) {
      out[i] = a * x[i] + y[i];
    }
  }

  dot(x: Float64Array, y: Float64Array): number {
    this.checkVec(x, y);
    if (x.length === 0) {
      throw new MyLibError(ErrorCode.INVALID_ARGUMENT, "dot requires non-empty vectors");
    }
    let sum = 0;
    for (let i = 0; i < x.length; i += 1) {
      sum += x[i] * y[i];
    }
    return sum;
  }

  matmul(
    aData: Float64Array,
    aShape: [number, number],
    bData: Float64Array,
    bShape: [number, number]
  ): Float64Array {
    const out = new Float64Array(aShape[0] * bShape[1]);
    this.matmulInto(aData, aShape, bData, bShape, out, [aShape[0], bShape[1]]);
    return out;
  }

  matmulInto(
    aData: Float64Array,
    aShape: [number, number],
    bData: Float64Array,
    bShape: [number, number],
    out: Float64Array,
    outShape: [number, number]
  ): void {
    const [ar, ac] = aShape;
    const [br, bc] = bShape;
    const [orows, ocols] = outShape;

    if (aData.length !== ar * ac || bData.length !== br * bc) {
      throw new MyLibError(ErrorCode.INVALID_ARGUMENT, "matrix data length does not match shape");
    }
    if (ac !== br) {
      throw new MyLibError(ErrorCode.SHAPE_MISMATCH, "a.cols must equal b.rows");
    }
    if (out.length !== orows * ocols || orows !== ar || ocols !== bc) {
      throw new MyLibError(ErrorCode.SHAPE_MISMATCH, "out shape must be [a.rows, b.cols]");
    }

    for (let i = 0; i < ar; i += 1) {
      for (let j = 0; j < bc; j += 1) {
        let sum = 0;
        for (let k = 0; k < ac; k += 1) {
          sum += aData[i * ac + k] * bData[k * bc + j];
        }
        out[i * bc + j] = sum;
      }
    }
  }

  sumAxis0(xData: Float64Array, xShape: [number, number]): Float64Array {
    const [rows, cols] = xShape;
    if (xData.length !== rows * cols) {
      throw new MyLibError(ErrorCode.INVALID_ARGUMENT, "x data length does not match shape");
    }
    const out = new Float64Array(cols);
    for (let r = 0; r < rows; r += 1) {
      for (let c = 0; c < cols; c += 1) {
        out[c] += xData[r * cols + c];
      }
    }
    return out;
  }

  boundingBox(points: Point2[]): [number, number, number, number] {
    if (points.length === 0) {
      throw new MyLibError(ErrorCode.INVALID_ARGUMENT, "points cannot be empty");
    }
    const xs = points.map((p) => p.x);
    const ys = points.map((p) => p.y);
    return [Math.min(...xs), Math.min(...ys), Math.max(...xs), Math.max(...ys)];
  }

  convexHull(points: Point2[]): Point2[] {
    const uniq = Array.from(new Map(points.map((p) => [`${p.x}:${p.y}`, p])).values()).sort(
      (a, b) => (a.x === b.x ? a.y - b.y : a.x - b.x)
    );
    if (uniq.length <= 1) {
      return uniq;
    }

    const lower: Point2[] = [];
    for (const p of uniq) {
      while (lower.length >= 2 && cross(lower[lower.length - 2], lower[lower.length - 1], p) <= 0) {
        lower.pop();
      }
      lower.push(p);
    }

    const upper: Point2[] = [];
    for (let i = uniq.length - 1; i >= 0; i -= 1) {
      const p = uniq[i];
      while (upper.length >= 2 && cross(upper[upper.length - 2], upper[upper.length - 1], p) <= 0) {
        upper.pop();
      }
      upper.push(p);
    }

    return lower.slice(0, -1).concat(upper.slice(0, -1));
  }

  convexHullInto(points: Point2[], out: Point2[]): number {
    const hull = this.convexHull(points);
    if (out.length < hull.length) {
      throw new MyLibError(ErrorCode.BUFFER_TOO_SMALL, "out buffer is too small for hull points");
    }
    for (let i = 0; i < hull.length; i += 1) {
      out[i] = hull[i];
    }
    return hull.length;
  }

  polygonContains(polygon: Point2[], point: Point2): boolean {
    const uniq = new Set(polygon.map((p) => `${p.x}:${p.y}`));
    if (uniq.size < 3) {
      throw new MyLibError(ErrorCode.INVALID_ARGUMENT, "polygon must have >=3 unique points");
    }

    let inside = false;
    for (let i = 0; i < polygon.length; i += 1) {
      const j = (i + 1) % polygon.length;
      const a = polygon[i];
      const b = polygon[j];
      const intersects =
        a.y > point.y !== b.y > point.y &&
        point.x < ((b.x - a.x) * (point.y - a.y)) / (b.y - a.y + 1e-15) + a.x;
      if (intersects) {
        inside = !inside;
      }
    }
    return inside;
  }

  private checkVec(x: Float64Array, y: Float64Array): void {
    if (x.length !== y.length) {
      throw new MyLibError(ErrorCode.SHAPE_MISMATCH, "x and y lengths must match");
    }
  }
}
