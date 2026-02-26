from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import numpy.typing as npt

from .backend_mock import MockBackend

FloatArray = npt.NDArray[np.float64]


@dataclass(frozen=True)
class Point2:
    x: float
    y: float


class MyLibClient:
    """Stable client API; backend can be swapped on other branches."""

    def __init__(self) -> None:
        self._backend = MockBackend()

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
        return self._backend.bounding_box(points)

    def convex_hull(self, points: list[Point2]) -> list[Point2]:
        return self._backend.convex_hull(points)

    def convex_hull_into(self, points: list[Point2], out_points: list[Point2]) -> int:
        return self._backend.convex_hull_into(points, out_points)

    def polygon_contains(self, polygon: list[Point2], point: Point2) -> bool:
        return self._backend.polygon_contains(polygon, point)
