#![allow(clippy::not_unsafe_ptr_arg_deref)]

use std::cell::RefCell;
use std::ffi::CString;
use std::os::raw::c_char;

use mylib_core::{self as core, MyLibError, Point2};

#[repr(C)]
#[allow(non_camel_case_types)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum mylib_status {
    MYLIB_OK = 0,
    MYLIB_INVALID_ARGUMENT = 1,
    MYLIB_SHAPE_MISMATCH = 2,
    MYLIB_DIV_BY_ZERO = 3,
    MYLIB_BUFFER_TOO_SMALL = 4,
    MYLIB_INTERNAL_ERROR = 5,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct mylib_point2d {
    pub x: f64,
    pub y: f64,
}

thread_local! {
    static LAST_ERROR: RefCell<CString> = RefCell::new(CString::new("OK").expect("valid cstring"));
}

fn set_last_error(msg: &str) {
    let sanitized = msg.replace('\0', " ");
    let cstr = CString::new(sanitized)
        .unwrap_or_else(|_| CString::new("INTERNAL_ERROR: invalid error").unwrap());
    LAST_ERROR.with(|cell| {
        *cell.borrow_mut() = cstr;
    });
}

fn status_from_error(err: &MyLibError) -> mylib_status {
    match err {
        MyLibError::InvalidArgument(_) => mylib_status::MYLIB_INVALID_ARGUMENT,
        MyLibError::ShapeMismatch(_) => mylib_status::MYLIB_SHAPE_MISMATCH,
        MyLibError::DivByZero(_) => mylib_status::MYLIB_DIV_BY_ZERO,
        MyLibError::BufferTooSmall(_) => mylib_status::MYLIB_BUFFER_TOO_SMALL,
        MyLibError::InternalError(_) => mylib_status::MYLIB_INTERNAL_ERROR,
    }
}

fn ptr_to_slice<'a>(ptr: *const f64, len: usize) -> Result<&'a [f64], MyLibError> {
    if ptr.is_null() {
        return Err(MyLibError::InvalidArgument("null pointer".to_string()));
    }
    // SAFETY: caller provides valid pointer with len elements.
    Ok(unsafe { std::slice::from_raw_parts(ptr, len) })
}

fn ptr_to_slice_mut<'a>(ptr: *mut f64, len: usize) -> Result<&'a mut [f64], MyLibError> {
    if ptr.is_null() {
        return Err(MyLibError::InvalidArgument("null pointer".to_string()));
    }
    // SAFETY: caller provides valid mutable pointer with len elements.
    Ok(unsafe { std::slice::from_raw_parts_mut(ptr, len) })
}

fn points_to_slice<'a>(
    ptr: *const mylib_point2d,
    len: usize,
) -> Result<&'a [mylib_point2d], MyLibError> {
    if ptr.is_null() {
        return Err(MyLibError::InvalidArgument(
            "null point pointer".to_string(),
        ));
    }
    // SAFETY: caller provides valid pointer with len elements.
    Ok(unsafe { std::slice::from_raw_parts(ptr, len) })
}

fn points_to_slice_mut<'a>(
    ptr: *mut mylib_point2d,
    len: usize,
) -> Result<&'a mut [mylib_point2d], MyLibError> {
    if ptr.is_null() {
        return Err(MyLibError::InvalidArgument(
            "null point output pointer".to_string(),
        ));
    }
    // SAFETY: caller provides valid mutable pointer with len elements.
    Ok(unsafe { std::slice::from_raw_parts_mut(ptr, len) })
}

fn into_core_points(points: &[mylib_point2d]) -> Vec<Point2> {
    points.iter().map(|p| Point2 { x: p.x, y: p.y }).collect()
}

fn write_error(err: MyLibError) -> mylib_status {
    set_last_error(&err.to_string());
    status_from_error(&err)
}

fn write_internal_panic() -> mylib_status {
    set_last_error("INTERNAL_ERROR: panic across FFI boundary prevented");
    mylib_status::MYLIB_INTERNAL_ERROR
}

fn ffi_guard<F>(f: F) -> mylib_status
where
    F: FnOnce() -> Result<(), MyLibError>,
{
    match std::panic::catch_unwind(std::panic::AssertUnwindSafe(f)) {
        Ok(Ok(())) => {
            set_last_error("OK");
            mylib_status::MYLIB_OK
        }
        Ok(Err(err)) => write_error(err),
        Err(_) => write_internal_panic(),
    }
}

#[no_mangle]
pub extern "C" fn mylib_last_error_message() -> *const c_char {
    LAST_ERROR.with(|cell| cell.borrow().as_ptr())
}

#[no_mangle]
pub extern "C" fn mylib_add(a: f64, b: f64, out: *mut f64) -> mylib_status {
    ffi_guard(|| {
        let out_ref = ptr_to_slice_mut(out, 1)?;
        out_ref[0] = core::add(a, b);
        Ok(())
    })
}

#[no_mangle]
pub extern "C" fn mylib_div(a: f64, b: f64, out: *mut f64) -> mylib_status {
    ffi_guard(|| {
        let out_ref = ptr_to_slice_mut(out, 1)?;
        out_ref[0] = core::div(a, b)?;
        Ok(())
    })
}

#[no_mangle]
pub extern "C" fn mylib_axpy(
    a: f64,
    x: *const f64,
    x_len: usize,
    y: *const f64,
    y_len: usize,
    out: *mut f64,
    out_len: usize,
) -> mylib_status {
    ffi_guard(|| {
        let x = ptr_to_slice(x, x_len)?;
        let y = ptr_to_slice(y, y_len)?;
        let out = ptr_to_slice_mut(out, out_len)?;
        core::axpy_into(a, x, y, out)
    })
}

#[no_mangle]
pub extern "C" fn mylib_dot(
    x: *const f64,
    x_len: usize,
    y: *const f64,
    y_len: usize,
    out: *mut f64,
) -> mylib_status {
    ffi_guard(|| {
        let x = ptr_to_slice(x, x_len)?;
        let y = ptr_to_slice(y, y_len)?;
        let out_ref = ptr_to_slice_mut(out, 1)?;
        out_ref[0] = core::dot(x, y)?;
        Ok(())
    })
}

#[allow(clippy::too_many_arguments)]
#[no_mangle]
pub extern "C" fn mylib_matmul(
    a: *const f64,
    a_rows: usize,
    a_cols: usize,
    b: *const f64,
    b_rows: usize,
    b_cols: usize,
    out: *mut f64,
    out_rows: usize,
    out_cols: usize,
) -> mylib_status {
    ffi_guard(|| {
        let a = ptr_to_slice(a, a_rows.saturating_mul(a_cols))?;
        let b = ptr_to_slice(b, b_rows.saturating_mul(b_cols))?;
        let out = ptr_to_slice_mut(out, out_rows.saturating_mul(out_cols))?;
        core::matmul_into(
            a, a_rows, a_cols, b, b_rows, b_cols, out, out_rows, out_cols,
        )
    })
}

#[no_mangle]
pub extern "C" fn mylib_sum_axis0(
    x: *const f64,
    rows: usize,
    cols: usize,
    out: *mut f64,
    out_len: usize,
) -> mylib_status {
    ffi_guard(|| {
        let x = ptr_to_slice(x, rows.saturating_mul(cols))?;
        let out = ptr_to_slice_mut(out, out_len)?;
        core::sum_axis0_into(x, rows, cols, out)
    })
}

#[no_mangle]
pub extern "C" fn mylib_bounding_box(
    points: *const mylib_point2d,
    n_points: usize,
    xmin: *mut f64,
    ymin: *mut f64,
    xmax: *mut f64,
    ymax: *mut f64,
) -> mylib_status {
    ffi_guard(|| {
        let points = points_to_slice(points, n_points)?;
        let core_points = into_core_points(points);

        let xmn = ptr_to_slice_mut(xmin, 1)?;
        let ymn = ptr_to_slice_mut(ymin, 1)?;
        let xmx = ptr_to_slice_mut(xmax, 1)?;
        let ymx = ptr_to_slice_mut(ymax, 1)?;

        let (a, b, c, d) = core::bounding_box(&core_points)?;
        xmn[0] = a;
        ymn[0] = b;
        xmx[0] = c;
        ymx[0] = d;
        Ok(())
    })
}

#[no_mangle]
pub extern "C" fn mylib_convex_hull(
    points: *const mylib_point2d,
    n_points: usize,
    out_points: *mut mylib_point2d,
    out_capacity: usize,
    out_n: *mut usize,
) -> mylib_status {
    ffi_guard(|| {
        if out_n.is_null() {
            return Err(MyLibError::InvalidArgument("out_n is null".to_string()));
        }
        let points = points_to_slice(points, n_points)?;
        let core_points = into_core_points(points);
        let out = points_to_slice_mut(out_points, out_capacity)?;
        let mut tmp = vec![Point2 { x: 0.0, y: 0.0 }; out_capacity];
        let n = core::convex_hull_into(&core_points, &mut tmp)?;
        for (idx, p) in tmp.iter().take(n).enumerate() {
            out[idx] = mylib_point2d { x: p.x, y: p.y };
        }

        // SAFETY: checked for null above.
        unsafe {
            *out_n = n;
        }
        Ok(())
    })
}

#[no_mangle]
pub extern "C" fn mylib_polygon_contains(
    polygon: *const mylib_point2d,
    polygon_points: usize,
    p: mylib_point2d,
    out_contains: *mut bool,
) -> mylib_status {
    ffi_guard(|| {
        if out_contains.is_null() {
            return Err(MyLibError::InvalidArgument(
                "out_contains pointer is null".to_string(),
            ));
        }

        let polygon = points_to_slice(polygon, polygon_points)?;
        let core_polygon = into_core_points(polygon);
        let contains = core::polygon_contains(&core_polygon, Point2 { x: p.x, y: p.y })?;
        // SAFETY: checked null above.
        unsafe {
            *out_contains = contains;
        }
        Ok(())
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ffi_scalar_smoke() {
        let mut out = 0.0f64;
        let s = mylib_add(2.0, 3.0, &mut out as *mut f64);
        assert_eq!(s, mylib_status::MYLIB_OK);
        assert_eq!(out, 5.0);
    }

    #[test]
    fn ffi_error_message() {
        let mut out = 0.0f64;
        let s = mylib_div(1.0, 0.0, &mut out as *mut f64);
        assert_eq!(s, mylib_status::MYLIB_DIV_BY_ZERO);
        let msg = unsafe { std::ffi::CStr::from_ptr(mylib_last_error_message()) };
        assert!(msg.to_str().unwrap_or("").contains("DIV_BY_ZERO"));
    }
}
