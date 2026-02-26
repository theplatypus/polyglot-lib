from __future__ import annotations

import numpy as np
import pytest

from examples.api import MyLibClient, Point2


def test_scalars() -> None:
    lib = MyLibClient()
    assert lib.add(2.0, 5.0) == 7.0
    assert lib.div(9.0, 3.0) == 3.0


def test_div_by_zero() -> None:
    lib = MyLibClient()
    with pytest.raises(ZeroDivisionError):
        lib.div(1.0, 0.0)


def test_axpy_and_dot() -> None:
    lib = MyLibClient()
    x = np.array([1.0, 2.0, 3.0], dtype=np.float64)
    y = np.array([0.5, 1.5, 2.5], dtype=np.float64)

    np.testing.assert_allclose(lib.axpy(2.0, x, y), 2.0 * x + y)
    assert lib.dot(x, y) == pytest.approx(float(np.dot(x, y)))


def test_shape_mismatch() -> None:
    lib = MyLibClient()
    with pytest.raises(ValueError):
        lib.axpy(1.0, np.array([1.0]), np.array([1.0, 2.0]))


def test_matmul_and_sum_axis0() -> None:
    lib = MyLibClient()
    a = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float64)
    b = np.array([[5.0, 6.0], [7.0, 8.0]], dtype=np.float64)

    np.testing.assert_allclose(lib.matmul(a, b), a @ b)
    np.testing.assert_allclose(lib.sum_axis0(a), np.sum(a, axis=0))


def test_geometry() -> None:
    lib = MyLibClient()
    points = [
        Point2(0.0, 0.0),
        Point2(2.0, 0.0),
        Point2(2.0, 2.0),
        Point2(0.0, 2.0),
        Point2(1.0, 1.0),
    ]

    bbox = lib.bounding_box(points)
    assert bbox == (0.0, 0.0, 2.0, 2.0)

    hull = lib.convex_hull(points)
    assert len(hull) == 4

    polygon = [Point2(0.0, 0.0), Point2(2.0, 0.0), Point2(2.0, 2.0), Point2(0.0, 2.0)]
    assert lib.polygon_contains(polygon, Point2(1.0, 1.0)) is True
    assert lib.polygon_contains(polygon, Point2(3.0, 1.0)) is False


def test_polygon_invalid() -> None:
    lib = MyLibClient()
    with pytest.raises(ValueError):
        lib.polygon_contains([Point2(0.0, 0.0), Point2(1.0, 1.0)], Point2(0.1, 0.1))
