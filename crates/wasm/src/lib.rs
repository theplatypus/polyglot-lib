use mylib_core::{self as core, MyLibError, Point2};
use wasm_bindgen::prelude::*;

fn map_error(err: MyLibError) -> JsValue {
    JsValue::from_str(&err.to_string())
}

fn decode_points(flat: &[f64]) -> Result<Vec<Point2>, JsValue> {
    if !flat.len().is_multiple_of(2) {
        return Err(JsValue::from_str(
            "INVALID_ARGUMENT: point buffer length must be even",
        ));
    }

    Ok(flat
        .chunks_exact(2)
        .map(|c| Point2 { x: c[0], y: c[1] })
        .collect())
}

fn encode_points(points: &[Point2]) -> Vec<f64> {
    let mut out = Vec::with_capacity(points.len() * 2);
    for p in points {
        out.push(p.x);
        out.push(p.y);
    }
    out
}

#[wasm_bindgen]
pub fn add(a: f64, b: f64) -> f64 {
    core::add(a, b)
}

#[wasm_bindgen]
pub fn div(a: f64, b: f64) -> Result<f64, JsValue> {
    core::div(a, b).map_err(map_error)
}

#[wasm_bindgen]
pub fn axpy(a: f64, x: &[f64], y: &[f64]) -> Result<Vec<f64>, JsValue> {
    core::axpy(a, x, y).map_err(map_error)
}

#[wasm_bindgen]
pub fn axpy_into(a: f64, x: &[f64], y: &[f64], out: &mut [f64]) -> Result<(), JsValue> {
    core::axpy_into(a, x, y, out).map_err(map_error)
}

#[wasm_bindgen]
pub fn dot(x: &[f64], y: &[f64]) -> Result<f64, JsValue> {
    core::dot(x, y).map_err(map_error)
}

#[wasm_bindgen]
pub fn matmul(
    a_data: &[f64],
    a_rows: usize,
    a_cols: usize,
    b_data: &[f64],
    b_rows: usize,
    b_cols: usize,
) -> Result<Vec<f64>, JsValue> {
    core::matmul(a_data, a_rows, a_cols, b_data, b_rows, b_cols).map_err(map_error)
}

#[allow(clippy::too_many_arguments)]
#[wasm_bindgen]
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
) -> Result<(), JsValue> {
    core::matmul_into(
        a_data, a_rows, a_cols, b_data, b_rows, b_cols, out, out_rows, out_cols,
    )
    .map_err(map_error)
}

#[wasm_bindgen]
pub fn sum_axis0(x_data: &[f64], rows: usize, cols: usize) -> Result<Vec<f64>, JsValue> {
    core::sum_axis0(x_data, rows, cols).map_err(map_error)
}

#[wasm_bindgen]
pub fn bounding_box(points_flat: &[f64]) -> Result<Vec<f64>, JsValue> {
    let points = decode_points(points_flat)?;
    let (xmin, ymin, xmax, ymax) = core::bounding_box(&points).map_err(map_error)?;
    Ok(vec![xmin, ymin, xmax, ymax])
}

#[wasm_bindgen]
pub fn convex_hull(points_flat: &[f64]) -> Result<Vec<f64>, JsValue> {
    let points = decode_points(points_flat)?;
    Ok(encode_points(&core::convex_hull(&points)))
}

#[wasm_bindgen]
pub fn convex_hull_into(points_flat: &[f64], out_flat: &mut [f64]) -> Result<usize, JsValue> {
    if !out_flat.len().is_multiple_of(2) {
        return Err(JsValue::from_str(
            "INVALID_ARGUMENT: output point buffer length must be even",
        ));
    }
    let points = decode_points(points_flat)?;
    let mut out = vec![Point2 { x: 0.0, y: 0.0 }; out_flat.len() / 2];
    let count = core::convex_hull_into(&points, &mut out).map_err(map_error)?;
    for (idx, p) in out.iter().take(count).enumerate() {
        out_flat[idx * 2] = p.x;
        out_flat[idx * 2 + 1] = p.y;
    }
    Ok(count)
}

#[wasm_bindgen]
pub fn polygon_contains(polygon_flat: &[f64], point_x: f64, point_y: f64) -> Result<bool, JsValue> {
    let polygon = decode_points(polygon_flat)?;
    core::polygon_contains(
        &polygon,
        Point2 {
            x: point_x,
            y: point_y,
        },
    )
    .map_err(map_error)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn smoke_native() {
        assert_eq!(add(2.0, 3.0), 5.0);
        assert_eq!(div(9.0, 3.0).unwrap(), 3.0);
    }
}

#[cfg(all(test, target_arch = "wasm32"))]
mod wasm_tests;
