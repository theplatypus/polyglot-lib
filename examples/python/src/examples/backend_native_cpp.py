from __future__ import annotations

from collections.abc import Sequence
from dataclasses import dataclass

import numpy as np
import numpy.typing as npt

try:
    import mylib_cpp
except ImportError as exc:
    raise ImportError(
        "mylib_cpp is not installed. Install a wheel from GitHub Releases or build/install it "
        "locally from cpp/pybind."
    ) from exc

FloatArray = npt.NDArray[np.float64]


@dataclass(frozen=True)
class Point2:
    x: float
    y: float


class NativeCppBackend:
    def add(self, a: float, b: float) -> float:
        return float(mylib_cpp.add(a, b))

    def div(self, a: float, b: float) -> float:
        return float(mylib_cpp.div(a, b))

    def axpy(self, a: float, x: FloatArray, y: FloatArray) -> FloatArray:
        return np.asarray(mylib_cpp.axpy(a, x, y), dtype=np.float64)

    def axpy_into(self, a: float, x: FloatArray, y: FloatArray, out: FloatArray) -> None:
        mylib_cpp.axpy_into(a, x, y, out)

    def dot(self, x: FloatArray, y: FloatArray) -> float:
        return float(mylib_cpp.dot(x, y))

    def matmul(self, a: FloatArray, b: FloatArray) -> FloatArray:
        return np.asarray(mylib_cpp.matmul(a, b), dtype=np.float64)

    def matmul_into(self, a: FloatArray, b: FloatArray, out: FloatArray) -> None:
        mylib_cpp.matmul_into(a, b, out)

    def sum_axis0(self, x: FloatArray) -> FloatArray:
        return np.asarray(mylib_cpp.sum_axis0(x), dtype=np.float64)

    def bounding_box(self, points: Sequence[Point2]) -> tuple[float, float, float, float]:
        return tuple(float(v) for v in mylib_cpp.bounding_box(self._to_points_array(points)))

    def convex_hull(self, points: Sequence[Point2]) -> list[Point2]:
        hull = np.asarray(mylib_cpp.convex_hull(self._to_points_array(points)), dtype=np.float64)
        return [Point2(float(p[0]), float(p[1])) for p in hull]

    def convex_hull_into(self, points: Sequence[Point2], out_points: list[Point2]) -> int:
        out = np.zeros((len(out_points), 2), dtype=np.float64)
        n = int(mylib_cpp.convex_hull_into(self._to_points_array(points), out))
        for i in range(n):
            out_points[i] = Point2(float(out[i, 0]), float(out[i, 1]))
        return n

    def polygon_contains(self, polygon: Sequence[Point2], point: Point2) -> bool:
        return bool(mylib_cpp.polygon_contains(self._to_points_array(polygon), point.x, point.y))

    def _to_points_array(self, points: Sequence[Point2]) -> npt.NDArray[np.float64]:
        arr = np.array([[p.x, p.y] for p in points], dtype=np.float64)
        if arr.ndim != 2:
            return np.empty((0, 2), dtype=np.float64)
        return arr
