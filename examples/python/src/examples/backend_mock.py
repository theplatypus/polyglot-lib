from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import numpy.typing as npt

FloatArray = npt.NDArray[np.float64]


@dataclass(frozen=True)
class Point2:
    x: float
    y: float


def _cross(o: Point2, a: Point2, b: Point2) -> float:
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x)


class MockBackend:
    def add(self, a: float, b: float) -> float:
        return float(a + b)

    def div(self, a: float, b: float) -> float:
        if b == 0.0:
            raise ZeroDivisionError("DIV_BY_ZERO: denominator must be non-zero")
        return float(a / b)

    def axpy(self, a: float, x: FloatArray, y: FloatArray) -> FloatArray:
        self._check_vec_shapes(x, y)
        return a * x + y

    def axpy_into(self, a: float, x: FloatArray, y: FloatArray, out: FloatArray) -> None:
        self._check_vec_shapes(x, y)
        if out.shape != x.shape:
            raise ValueError("SHAPE_MISMATCH: out must match x/y length")
        out[:] = a * x + y

    def dot(self, x: FloatArray, y: FloatArray) -> float:
        self._check_vec_shapes(x, y)
        if x.size == 0:
            raise ValueError("INVALID_ARGUMENT: dot requires non-empty vectors")
        return float(np.dot(x, y))

    def matmul(self, a: FloatArray, b: FloatArray) -> FloatArray:
        self._check_matmul_shapes(a, b)
        return a @ b

    def matmul_into(self, a: FloatArray, b: FloatArray, out: FloatArray) -> None:
        self._check_matmul_shapes(a, b)
        m, _ = a.shape
        _, n = b.shape
        if out.shape != (m, n):
            raise ValueError("SHAPE_MISMATCH: out must be (a.rows, b.cols)")
        out[:] = a @ b

    def sum_axis0(self, x: FloatArray) -> FloatArray:
        if x.ndim != 2:
            raise ValueError("INVALID_ARGUMENT: sum_axis0 expects 2D matrix")
        return np.sum(x, axis=0)

    def bounding_box(self, points: list[Point2]) -> tuple[float, float, float, float]:
        if not points:
            raise ValueError("INVALID_ARGUMENT: points cannot be empty")
        xs = [p.x for p in points]
        ys = [p.y for p in points]
        return (min(xs), min(ys), max(xs), max(ys))

    def convex_hull(self, points: list[Point2]) -> list[Point2]:
        uniq = sorted(set(points), key=lambda p: (p.x, p.y))
        if len(uniq) <= 1:
            return uniq

        lower: list[Point2] = []
        for p in uniq:
            while len(lower) >= 2 and _cross(lower[-2], lower[-1], p) <= 0:
                lower.pop()
            lower.append(p)

        upper: list[Point2] = []
        for p in reversed(uniq):
            while len(upper) >= 2 and _cross(upper[-2], upper[-1], p) <= 0:
                upper.pop()
            upper.append(p)

        return lower[:-1] + upper[:-1]

    def convex_hull_into(self, points: list[Point2], out_points: list[Point2]) -> int:
        hull = self.convex_hull(points)
        if len(out_points) < len(hull):
            raise RuntimeError("BUFFER_TOO_SMALL: out_points capacity is insufficient")
        for i, p in enumerate(hull):
            out_points[i] = p
        return len(hull)

    def polygon_contains(self, polygon: list[Point2], point: Point2) -> bool:
        uniq = {(p.x, p.y) for p in polygon}
        if len(uniq) < 3:
            raise ValueError("INVALID_ARGUMENT: polygon must have >=3 unique points")

        ring = polygon[:]
        if ring[0] != ring[-1]:
            ring.append(ring[0])

        inside = False
        for i in range(len(ring) - 1):
            a = ring[i]
            b = ring[i + 1]
            intersects = ((a.y > point.y) != (b.y > point.y)) and (
                point.x < (b.x - a.x) * (point.y - a.y) / ((b.y - a.y) + 1e-15) + a.x
            )
            if intersects:
                inside = not inside
        return inside

    def _check_vec_shapes(self, x: FloatArray, y: FloatArray) -> None:
        if x.ndim != 1 or y.ndim != 1:
            raise ValueError("INVALID_ARGUMENT: x and y must be 1D")
        if x.shape != y.shape:
            raise ValueError("SHAPE_MISMATCH: x and y lengths must match")

    def _check_matmul_shapes(self, a: FloatArray, b: FloatArray) -> None:
        if a.ndim != 2 or b.ndim != 2:
            raise ValueError("INVALID_ARGUMENT: matmul expects 2D matrices")
        if a.shape[1] != b.shape[0]:
            raise ValueError("SHAPE_MISMATCH: a.cols must equal b.rows")
