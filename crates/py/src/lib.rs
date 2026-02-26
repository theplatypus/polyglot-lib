#![allow(deprecated)]

use mylib_core::{self as core, MyLibError, Point2};
use numpy::ndarray::{Array1, Array2, Axis};
use numpy::{
    IntoPyArray, PyArray1, PyArray2, PyReadonlyArray1, PyReadonlyArray2, PyReadwriteArray1,
    PyReadwriteArray2, PyUntypedArrayMethods,
};
use pyo3::exceptions::{PyRuntimeError, PyValueError, PyZeroDivisionError};
use pyo3::prelude::*;

fn map_error(err: MyLibError) -> PyErr {
    match err {
        MyLibError::InvalidArgument(msg) => PyValueError::new_err(msg),
        MyLibError::ShapeMismatch(msg) => PyValueError::new_err(msg),
        MyLibError::DivByZero(msg) => PyZeroDivisionError::new_err(msg),
        MyLibError::BufferTooSmall(msg) => PyRuntimeError::new_err(msg),
        MyLibError::InternalError(msg) => PyRuntimeError::new_err(msg),
    }
}

#[pyfunction]
fn add(a: f64, b: f64) -> f64 {
    core::add(a, b)
}

#[pyfunction]
fn div(a: f64, b: f64) -> PyResult<f64> {
    core::div(a, b).map_err(map_error)
}

#[pyfunction]
fn axpy<'py>(
    py: Python<'py>,
    a: f64,
    x: PyReadonlyArray1<'py, f64>,
    y: PyReadonlyArray1<'py, f64>,
) -> PyResult<&'py PyArray1<f64>> {
    let out = core::axpy(a, x.as_slice()?, y.as_slice()?).map_err(map_error)?;
    Ok(out.into_pyarray(py))
}

#[pyfunction]
fn axpy_into<'py>(
    a: f64,
    x: PyReadonlyArray1<'py, f64>,
    y: PyReadonlyArray1<'py, f64>,
    mut out: PyReadwriteArray1<'py, f64>,
) -> PyResult<()> {
    core::axpy_into(a, x.as_slice()?, y.as_slice()?, out.as_slice_mut()?).map_err(map_error)
}

#[pyfunction]
fn dot<'py>(x: PyReadonlyArray1<'py, f64>, y: PyReadonlyArray1<'py, f64>) -> PyResult<f64> {
    core::dot(x.as_slice()?, y.as_slice()?).map_err(map_error)
}

#[pyfunction]
fn matmul<'py>(
    py: Python<'py>,
    a: PyReadonlyArray2<'py, f64>,
    b: PyReadonlyArray2<'py, f64>,
) -> PyResult<&'py PyArray2<f64>> {
    let a_view = a.as_array();
    let b_view = b.as_array();
    let out = core::matmul(
        a.as_slice()?,
        a_view.shape()[0],
        a_view.shape()[1],
        b.as_slice()?,
        b_view.shape()[0],
        b_view.shape()[1],
    )
    .map_err(map_error)?;
    let out_arr = Array2::from_shape_vec((a_view.shape()[0], b_view.shape()[1]), out)
        .map_err(|e| PyRuntimeError::new_err(e.to_string()))?;
    Ok(out_arr.into_pyarray(py))
}

#[pyfunction]
fn matmul_into<'py>(
    a: PyReadonlyArray2<'py, f64>,
    b: PyReadonlyArray2<'py, f64>,
    mut out: PyReadwriteArray2<'py, f64>,
) -> PyResult<()> {
    let a_view = a.as_array();
    let b_view = b.as_array();
    let out_rows = out.shape()[0];
    let out_cols = out.shape()[1];
    core::matmul_into(
        a.as_slice()?,
        a_view.shape()[0],
        a_view.shape()[1],
        b.as_slice()?,
        b_view.shape()[0],
        b_view.shape()[1],
        out.as_slice_mut()?,
        out_rows,
        out_cols,
    )
    .map_err(map_error)
}

#[pyfunction]
fn sum_axis0<'py>(py: Python<'py>, x: PyReadonlyArray2<'py, f64>) -> PyResult<&'py PyArray1<f64>> {
    let view = x.as_array();
    let out =
        core::sum_axis0(x.as_slice()?, view.shape()[0], view.shape()[1]).map_err(map_error)?;
    Ok(Array1::from(out).into_pyarray(py))
}

fn to_points<'py>(points: PyReadonlyArray2<'py, f64>) -> PyResult<Vec<Point2>> {
    let view = points.as_array();
    if view.shape().len() != 2 || view.shape()[1] != 2 {
        return Err(PyValueError::new_err("points must have shape (n,2)"));
    }
    Ok(view
        .axis_iter(Axis(0))
        .map(|row| Point2 {
            x: row[0],
            y: row[1],
        })
        .collect())
}

fn points_to_array(points: Vec<Point2>) -> PyResult<Array2<f64>> {
    let mut flat = Vec::with_capacity(points.len() * 2);
    for p in points {
        flat.push(p.x);
        flat.push(p.y);
    }
    Array2::from_shape_vec((flat.len() / 2, 2), flat)
        .map_err(|e| PyRuntimeError::new_err(e.to_string()))
}

#[pyfunction]
fn bounding_box<'py>(points: PyReadonlyArray2<'py, f64>) -> PyResult<(f64, f64, f64, f64)> {
    core::bounding_box(&to_points(points)?).map_err(map_error)
}

#[pyfunction]
fn convex_hull<'py>(
    py: Python<'py>,
    points: PyReadonlyArray2<'py, f64>,
) -> PyResult<&'py PyArray2<f64>> {
    let hull = core::convex_hull(&to_points(points)?);
    let arr = points_to_array(hull)?;
    Ok(arr.into_pyarray(py))
}

#[pyfunction]
fn convex_hull_into<'py>(
    points: PyReadonlyArray2<'py, f64>,
    mut out: PyReadwriteArray2<'py, f64>,
) -> PyResult<usize> {
    let out_rows = out.shape()[0];
    let out_cols = out.shape()[1];
    if out_cols != 2 {
        return Err(PyValueError::new_err("out must have shape (n,2)"));
    }
    let mut out_points = vec![Point2 { x: 0.0, y: 0.0 }; out_rows];
    let count = core::convex_hull_into(&to_points(points)?, &mut out_points).map_err(map_error)?;

    let out_slice = out.as_slice_mut()?;
    for (i, p) in out_points.iter().take(count).enumerate() {
        out_slice[i * 2] = p.x;
        out_slice[i * 2 + 1] = p.y;
    }
    Ok(count)
}

#[pyfunction]
fn polygon_contains<'py>(
    polygon: PyReadonlyArray2<'py, f64>,
    point_x: f64,
    point_y: f64,
) -> PyResult<bool> {
    core::polygon_contains(
        &to_points(polygon)?,
        Point2 {
            x: point_x,
            y: point_y,
        },
    )
    .map_err(map_error)
}

#[pymodule]
fn mylib_rs(_py: Python<'_>, m: &PyModule) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(add, m)?)?;
    m.add_function(wrap_pyfunction!(div, m)?)?;
    m.add_function(wrap_pyfunction!(axpy, m)?)?;
    m.add_function(wrap_pyfunction!(axpy_into, m)?)?;
    m.add_function(wrap_pyfunction!(dot, m)?)?;
    m.add_function(wrap_pyfunction!(matmul, m)?)?;
    m.add_function(wrap_pyfunction!(matmul_into, m)?)?;
    m.add_function(wrap_pyfunction!(sum_axis0, m)?)?;
    m.add_function(wrap_pyfunction!(bounding_box, m)?)?;
    m.add_function(wrap_pyfunction!(convex_hull, m)?)?;
    m.add_function(wrap_pyfunction!(convex_hull_into, m)?)?;
    m.add_function(wrap_pyfunction!(polygon_contains, m)?)?;
    Ok(())
}
