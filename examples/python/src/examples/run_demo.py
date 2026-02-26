from __future__ import annotations

import numpy as np

from .api import MyLibClient, Point2


def main() -> None:
    lib = MyLibClient()

    print("add:", lib.add(2.0, 3.5))
    print("div:", lib.div(9.0, 3.0))

    x = np.array([1.0, 2.0, 3.0], dtype=np.float64)
    y = np.array([0.5, 1.5, 2.5], dtype=np.float64)
    print("axpy:", lib.axpy(2.0, x, y))

    out = np.empty_like(x)
    lib.axpy_into(2.0, x, y, out)
    print("axpy_into:", out)
    print("dot:", lib.dot(x, y))

    a = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float64)
    b = np.array([[5.0, 6.0], [7.0, 8.0]], dtype=np.float64)
    print("matmul:\n", lib.matmul(a, b))

    out_m = np.empty((2, 2), dtype=np.float64)
    lib.matmul_into(a, b, out_m)
    print("matmul_into:\n", out_m)

    print("sum_axis0:", lib.sum_axis0(a))

    points = [
        Point2(0.0, 0.0),
        Point2(1.0, 0.0),
        Point2(1.0, 1.0),
        Point2(0.0, 1.0),
        Point2(0.5, 0.5),
    ]
    print("bounding_box:", lib.bounding_box(points))
    hull = lib.convex_hull(points)
    print("convex_hull:", hull)

    out_hull = [Point2(0.0, 0.0)] * 16
    n = lib.convex_hull_into(points, out_hull)
    print("convex_hull_into count:", n, "points:", out_hull[:n])

    polygon = [Point2(0.0, 0.0), Point2(2.0, 0.0), Point2(2.0, 2.0), Point2(0.0, 2.0)]
    print("polygon_contains (inside):", lib.polygon_contains(polygon, Point2(1.0, 1.0)))
    print("polygon_contains (outside):", lib.polygon_contains(polygon, Point2(3.0, 1.0)))

    try:
        lib.div(1.0, 0.0)
    except ZeroDivisionError as exc:
        print("expected error:", exc)

    try:
        lib.axpy(1.0, np.array([1.0]), np.array([1.0, 2.0]))
    except ValueError as exc:
        print("expected shape error:", exc)


if __name__ == "__main__":
    main()
