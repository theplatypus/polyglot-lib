//! Core compute kernels for the polyglot example API.

use std::cmp::Ordering;

use thiserror::Error;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorCode {
    Ok,
    InvalidArgument,
    ShapeMismatch,
    DivByZero,
    BufferTooSmall,
    InternalError,
}

#[derive(Debug, Error, Clone, PartialEq)]
pub enum MyLibError {
    #[error("INVALID_ARGUMENT: {0}")]
    InvalidArgument(String),
    #[error("SHAPE_MISMATCH: {0}")]
    ShapeMismatch(String),
    #[error("DIV_BY_ZERO: {0}")]
    DivByZero(String),
    #[error("BUFFER_TOO_SMALL: {0}")]
    BufferTooSmall(String),
    #[error("INTERNAL_ERROR: {0}")]
    InternalError(String),
}

impl MyLibError {
    pub fn code(&self) -> ErrorCode {
        match self {
            Self::InvalidArgument(_) => ErrorCode::InvalidArgument,
            Self::ShapeMismatch(_) => ErrorCode::ShapeMismatch,
            Self::DivByZero(_) => ErrorCode::DivByZero,
            Self::BufferTooSmall(_) => ErrorCode::BufferTooSmall,
            Self::InternalError(_) => ErrorCode::InternalError,
        }
    }
}

pub type Result<T> = std::result::Result<T, MyLibError>;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Point2 {
    pub x: f64,
    pub y: f64,
}

pub fn add(a: f64, b: f64) -> f64 {
    a + b
}

pub fn div(a: f64, b: f64) -> Result<f64> {
    if b == 0.0 {
        return Err(MyLibError::DivByZero(
            "denominator must be non-zero".to_string(),
        ));
    }
    Ok(a / b)
}

pub fn axpy(a: f64, x: &[f64], y: &[f64]) -> Result<Vec<f64>> {
    check_same_len(x, y, "x and y lengths must match")?;
    Ok(x.iter().zip(y.iter()).map(|(xi, yi)| a * xi + yi).collect())
}

pub fn axpy_into(a: f64, x: &[f64], y: &[f64], out: &mut [f64]) -> Result<()> {
    check_same_len(x, y, "x and y lengths must match")?;
    check_same_len(x, out, "out length must match x and y")?;
    out.iter_mut()
        .zip(x.iter().zip(y.iter()))
        .for_each(|(o, (xi, yi))| *o = a * xi + yi);
    Ok(())
}

pub fn dot(x: &[f64], y: &[f64]) -> Result<f64> {
    check_same_len(x, y, "x and y lengths must match")?;
    if x.is_empty() {
        return Err(MyLibError::InvalidArgument(
            "dot requires non-empty vectors".to_string(),
        ));
    }
    Ok(x.iter().zip(y.iter()).map(|(xi, yi)| xi * yi).sum())
}

pub fn matmul(
    a_data: &[f64],
    a_rows: usize,
    a_cols: usize,
    b_data: &[f64],
    b_rows: usize,
    b_cols: usize,
) -> Result<Vec<f64>> {
    let mut out = vec![0.0; a_rows * b_cols];
    matmul_into(
        a_data, a_rows, a_cols, b_data, b_rows, b_cols, &mut out, a_rows, b_cols,
    )?;
    Ok(out)
}

#[allow(clippy::too_many_arguments)]
pub fn matmul_into(
    a_data: &[f64],
    a_rows: usize,
    a_cols: usize,
    b_data: &[f64],
    b_rows: usize,
    b_cols: usize,
    out: &mut [f64],
    out_rows: usize,
    out_cols: usize,
) -> Result<()> {
    check_matrix_shape(a_data, a_rows, a_cols, "a")?;
    check_matrix_shape(b_data, b_rows, b_cols, "b")?;
    if a_cols != b_rows {
        return Err(MyLibError::ShapeMismatch(
            "a.cols must equal b.rows".to_string(),
        ));
    }
    if out_rows != a_rows || out_cols != b_cols {
        return Err(MyLibError::ShapeMismatch(
            "out shape must be (a.rows, b.cols)".to_string(),
        ));
    }
    check_matrix_shape(out, out_rows, out_cols, "out")?;

    for r in 0..a_rows {
        for c in 0..b_cols {
            let mut acc = 0.0;
            for k in 0..a_cols {
                acc += a_data[r * a_cols + k] * b_data[k * b_cols + c];
            }
            out[r * out_cols + c] = acc;
        }
    }
    Ok(())
}

pub fn sum_axis0(x_data: &[f64], rows: usize, cols: usize) -> Result<Vec<f64>> {
    check_matrix_shape(x_data, rows, cols, "x")?;
    let mut out = vec![0.0; cols];
    sum_axis0_into(x_data, rows, cols, &mut out)?;
    Ok(out)
}

pub fn sum_axis0_into(x_data: &[f64], rows: usize, cols: usize, out: &mut [f64]) -> Result<()> {
    check_matrix_shape(x_data, rows, cols, "x")?;
    if out.len() != cols {
        return Err(MyLibError::ShapeMismatch(
            "out length must equal cols".to_string(),
        ));
    }
    out.fill(0.0);
    for r in 0..rows {
        for c in 0..cols {
            out[c] += x_data[r * cols + c];
        }
    }
    Ok(())
}

pub fn bounding_box(points: &[Point2]) -> Result<(f64, f64, f64, f64)> {
    if points.is_empty() {
        return Err(MyLibError::InvalidArgument(
            "points cannot be empty".to_string(),
        ));
    }

    let mut xmin = points[0].x;
    let mut ymin = points[0].y;
    let mut xmax = points[0].x;
    let mut ymax = points[0].y;

    for p in points.iter().skip(1) {
        xmin = xmin.min(p.x);
        ymin = ymin.min(p.y);
        xmax = xmax.max(p.x);
        ymax = ymax.max(p.y);
    }

    Ok((xmin, ymin, xmax, ymax))
}

pub fn convex_hull(points: &[Point2]) -> Vec<Point2> {
    let mut pts = unique_sorted(points);
    if pts.len() <= 1 {
        return pts;
    }

    let mut lower: Vec<Point2> = Vec::new();
    for p in &pts {
        while lower.len() >= 2 && cross(lower[lower.len() - 2], lower[lower.len() - 1], *p) <= 0.0 {
            lower.pop();
        }
        lower.push(*p);
    }

    let mut upper: Vec<Point2> = Vec::new();
    for p in pts.iter().rev() {
        while upper.len() >= 2 && cross(upper[upper.len() - 2], upper[upper.len() - 1], *p) <= 0.0 {
            upper.pop();
        }
        upper.push(*p);
    }

    lower.pop();
    upper.pop();
    lower.extend(upper);
    pts.clear();
    lower
}

pub fn convex_hull_into(points: &[Point2], out: &mut [Point2]) -> Result<usize> {
    let hull = convex_hull(points);
    if out.len() < hull.len() {
        return Err(MyLibError::BufferTooSmall(
            "out buffer is too small for hull points".to_string(),
        ));
    }
    out[..hull.len()].copy_from_slice(&hull);
    Ok(hull.len())
}

pub fn polygon_contains(polygon: &[Point2], point: Point2) -> Result<bool> {
    let unique_count = unique_sorted(polygon).len();
    if unique_count < 3 {
        return Err(MyLibError::InvalidArgument(
            "polygon must have >=3 unique points".to_string(),
        ));
    }

    let mut inside = false;
    let n = polygon.len();
    for i in 0..n {
        let j = (i + 1) % n;
        let a = polygon[i];
        let b = polygon[j];
        let intersects = ((a.y > point.y) != (b.y > point.y))
            && (point.x < (b.x - a.x) * (point.y - a.y) / ((b.y - a.y) + 1e-15) + a.x);
        if intersects {
            inside = !inside;
        }
    }
    Ok(inside)
}

fn check_same_len<T, U>(x: &[T], y: &[U], msg: &str) -> Result<()> {
    if x.len() != y.len() {
        return Err(MyLibError::ShapeMismatch(msg.to_string()));
    }
    Ok(())
}

fn check_matrix_shape(data: &[f64], rows: usize, cols: usize, name: &str) -> Result<()> {
    if rows.checked_mul(cols) != Some(data.len()) {
        return Err(MyLibError::InvalidArgument(format!(
            "{name} data length does not match shape"
        )));
    }
    Ok(())
}

fn cross(o: Point2, a: Point2, b: Point2) -> f64 {
    (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x)
}

fn unique_sorted(points: &[Point2]) -> Vec<Point2> {
    let mut pts = points.to_vec();
    pts.sort_by(|a, b| match a.x.total_cmp(&b.x) {
        Ordering::Equal => a.y.total_cmp(&b.y),
        ord => ord,
    });
    pts.dedup_by(|a, b| a.x == b.x && a.y == b.y);
    pts
}

#[cfg(test)]
mod tests {
    use approx::assert_abs_diff_eq;

    use super::*;

    #[test]
    fn scalar_ops() {
        assert_eq!(add(2.0, 3.0), 5.0);
        assert_eq!(div(6.0, 2.0).unwrap(), 3.0);
        assert!(matches!(div(1.0, 0.0), Err(MyLibError::DivByZero(_))));
    }

    #[test]
    fn vector_ops() {
        let x = [1.0, 2.0, 3.0];
        let y = [0.5, 1.5, 2.5];
        let out = axpy(2.0, &x, &y).unwrap();
        assert_eq!(out, vec![2.5, 5.5, 8.5]);
        assert_abs_diff_eq!(dot(&x, &y).unwrap(), 11.0);
    }

    #[test]
    fn tensor_ops() {
        let a = [1.0, 2.0, 3.0, 4.0];
        let b = [5.0, 6.0, 7.0, 8.0];
        let c = matmul(&a, 2, 2, &b, 2, 2).unwrap();
        assert_eq!(c, vec![19.0, 22.0, 43.0, 50.0]);
        let s = sum_axis0(&a, 2, 2).unwrap();
        assert_eq!(s, vec![4.0, 6.0]);
    }

    #[test]
    fn geometry_ops() {
        let points = vec![
            Point2 { x: 0.0, y: 0.0 },
            Point2 { x: 2.0, y: 0.0 },
            Point2 { x: 2.0, y: 2.0 },
            Point2 { x: 0.0, y: 2.0 },
            Point2 { x: 1.0, y: 1.0 },
        ];

        assert_eq!(bounding_box(&points).unwrap(), (0.0, 0.0, 2.0, 2.0));
        let hull = convex_hull(&points);
        assert_eq!(hull.len(), 4);

        let poly = vec![
            Point2 { x: 0.0, y: 0.0 },
            Point2 { x: 2.0, y: 0.0 },
            Point2 { x: 2.0, y: 2.0 },
            Point2 { x: 0.0, y: 2.0 },
        ];
        assert!(polygon_contains(&poly, Point2 { x: 1.0, y: 1.0 }).unwrap());
        assert!(!polygon_contains(&poly, Point2 { x: 3.0, y: 1.0 }).unwrap());
    }
}
