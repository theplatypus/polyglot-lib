from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import numpy.typing as npt

from .backend_native_cpp import NativeCppBackend, Point2 as NativePoint2

FloatArray = npt.NDArray[np.float64]


@dataclass(frozen=True)
class Point2:
    x: float
    y: float


class MyLibClient:
    """Stable client API backed by the compiled mylib_cpp extension."""

    def __init__(self) -> None:
        self._backend = NativeCppBackend()

    def add(self, a: float, b: float) -> float:
        return self._backend.add(a, b)

    def div(self, a: float, b: float) -> float:
        return self._backend.div(a, b)

    def axpy(self, a: float, x: FloatArray, y: FloatArray) -> FloatArray:
        return self._backend.axpy(a, x, y)

    def axpy_into(self, a: float, x: FloatArray, y: FloatArray, out: FloatArray) -> None:
        self._backend.axpy_into(a, x, y, out)

    def dot(self, x: FloatArray, y: FloatArray) -> float:
        return self._backend.dot(x, y)

    def matmul(self, a: FloatArray, b: FloatArray) -> FloatArray:
        return self._backend.matmul(a, b)

    def matmul_into(self, a: FloatArray, b: FloatArray, out: FloatArray) -> None:
        self._backend.matmul_into(a, b, out)

    def sum_axis0(self, x: FloatArray) -> FloatArray:
        return self._backend.sum_axis0(x)

    def bounding_box(self, points: list[Point2]) -> tuple[float, float, float, float]:
        return self._backend.bounding_box([NativePoint2(p.x, p.y) for p in points])

    def convex_hull(self, points: list[Point2]) -> list[Point2]:
        hull = self._backend.convex_hull([NativePoint2(p.x, p.y) for p in points])
        return [Point2(p.x, p.y) for p in hull]

    def convex_hull_into(self, points: list[Point2], out_points: list[Point2]) -> int:
        native_out = [NativePoint2(p.x, p.y) for p in out_points]
        count = self._backend.convex_hull_into(
            [NativePoint2(p.x, p.y) for p in points],
            native_out,
        )
        for i in range(count):
            out_points[i] = Point2(native_out[i].x, native_out[i].y)
        return count

    def polygon_contains(self, polygon: list[Point2], point: Point2) -> bool:
        return self._backend.polygon_contains(
            [NativePoint2(p.x, p.y) for p in polygon],
            NativePoint2(point.x, point.y),
        )
