#![cfg(target_arch = "wasm32")]

use wasm_bindgen_test::*;

use crate::{axpy, bounding_box, div, matmul, polygon_contains};

#[wasm_bindgen_test]
fn wasm_vector_and_scalar() {
    let out = axpy(2.0, &[1.0, 2.0], &[0.5, 1.5]).unwrap();
    assert_eq!(out, vec![2.5, 5.5]);
    assert!(div(1.0, 0.0).is_err());
}

#[wasm_bindgen_test]
fn wasm_matrix_and_geometry() {
    let c = matmul(&[1.0, 2.0, 3.0, 4.0], 2, 2, &[5.0, 6.0, 7.0, 8.0], 2, 2).unwrap();
    assert_eq!(c, vec![19.0, 22.0, 43.0, 50.0]);

    let bbox = bounding_box(&[0.0, 0.0, 2.0, 0.0, 2.0, 2.0, 0.0, 2.0]).unwrap();
    assert_eq!(bbox, vec![0.0, 0.0, 2.0, 2.0]);

    let inside = polygon_contains(&[0.0, 0.0, 2.0, 0.0, 2.0, 2.0, 0.0, 2.0], 1.0, 1.0).unwrap();
    assert!(inside);
}
