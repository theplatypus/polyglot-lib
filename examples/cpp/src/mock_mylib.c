#include "mylib.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_last_error[256] = "OK";

static mylib_status set_error(mylib_status status, const char *msg) {
  snprintf(g_last_error, sizeof(g_last_error), "%s", msg);
  return status;
}

const char *mylib_last_error_message(void) { return g_last_error; }

mylib_status mylib_add(double a, double b, double *out) {
  if (out == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out is NULL");
  }
  *out = a + b;
  return MYLIB_OK;
}

mylib_status mylib_div(double a, double b, double *out) {
  if (out == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out is NULL");
  }
  if (b == 0.0) {
    return set_error(MYLIB_DIV_BY_ZERO, "DIV_BY_ZERO: denominator must be non-zero");
  }
  *out = a / b;
  return MYLIB_OK;
}

mylib_status mylib_axpy(double a, const double *x, size_t x_len, const double *y, size_t y_len,
                       double *out, size_t out_len) {
  size_t i = 0;
  if (x == NULL || y == NULL || out == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: null vector pointer");
  }
  if (x_len != y_len || x_len != out_len) {
    return set_error(MYLIB_SHAPE_MISMATCH,
                     "SHAPE_MISMATCH: x/y/out lengths must match for axpy");
  }
  for (i = 0; i < x_len; i++) {
    out[i] = a * x[i] + y[i];
  }
  return MYLIB_OK;
}

mylib_status mylib_dot(const double *x, size_t x_len, const double *y, size_t y_len,
                      double *out) {
  size_t i = 0;
  double acc = 0.0;
  if (x == NULL || y == NULL || out == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: null pointer in dot");
  }
  if (x_len != y_len) {
    return set_error(MYLIB_SHAPE_MISMATCH, "SHAPE_MISMATCH: x and y length mismatch");
  }
  if (x_len == 0) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: dot requires non-empty vectors");
  }
  for (i = 0; i < x_len; i++) {
    acc += x[i] * y[i];
  }
  *out = acc;
  return MYLIB_OK;
}

mylib_status mylib_matmul(const double *a, size_t a_rows, size_t a_cols, const double *b,
                         size_t b_rows, size_t b_cols, double *out, size_t out_rows,
                         size_t out_cols) {
  size_t i = 0;
  size_t j = 0;
  size_t k = 0;
  if (a == NULL || b == NULL || out == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: null matrix pointer");
  }
  if (a_cols != b_rows) {
    return set_error(MYLIB_SHAPE_MISMATCH, "SHAPE_MISMATCH: a.cols must equal b.rows");
  }
  if (out_rows != a_rows || out_cols != b_cols) {
    return set_error(MYLIB_SHAPE_MISMATCH, "SHAPE_MISMATCH: out matrix has wrong shape");
  }
  for (i = 0; i < out_rows; i++) {
    for (j = 0; j < out_cols; j++) {
      double sum = 0.0;
      for (k = 0; k < a_cols; k++) {
        sum += a[i * a_cols + k] * b[k * b_cols + j];
      }
      out[i * out_cols + j] = sum;
    }
  }
  return MYLIB_OK;
}

mylib_status mylib_sum_axis0(const double *x, size_t rows, size_t cols, double *out,
                            size_t out_len) {
  size_t r = 0;
  size_t c = 0;
  if (x == NULL || out == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: null pointer in sum_axis0");
  }
  if (out_len != cols) {
    return set_error(MYLIB_SHAPE_MISMATCH, "SHAPE_MISMATCH: out length must equal cols");
  }
  for (c = 0; c < cols; c++) {
    out[c] = 0.0;
  }
  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++) {
      out[c] += x[r * cols + c];
    }
  }
  return MYLIB_OK;
}

mylib_status mylib_bounding_box(const mylib_point2d *points, size_t n_points, double *xmin,
                               double *ymin, double *xmax, double *ymax) {
  size_t i = 0;
  if (points == NULL || xmin == NULL || ymin == NULL || xmax == NULL || ymax == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: null pointer in bounding_box");
  }
  if (n_points == 0) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: points cannot be empty");
  }

  *xmin = *xmax = points[0].x;
  *ymin = *ymax = points[0].y;
  for (i = 1; i < n_points; i++) {
    if (points[i].x < *xmin) *xmin = points[i].x;
    if (points[i].x > *xmax) *xmax = points[i].x;
    if (points[i].y < *ymin) *ymin = points[i].y;
    if (points[i].y > *ymax) *ymax = points[i].y;
  }
  return MYLIB_OK;
}

static int point_cmp(const void *lhs, const void *rhs) {
  const mylib_point2d *a = (const mylib_point2d *)lhs;
  const mylib_point2d *b = (const mylib_point2d *)rhs;
  if (a->x < b->x) return -1;
  if (a->x > b->x) return 1;
  if (a->y < b->y) return -1;
  if (a->y > b->y) return 1;
  return 0;
}

static double cross(const mylib_point2d o, const mylib_point2d a, const mylib_point2d b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

mylib_status mylib_convex_hull(const mylib_point2d *points, size_t n_points,
                              mylib_point2d *out_points, size_t out_capacity,
                              size_t *out_n) {
  mylib_point2d buf[512];
  mylib_point2d hull[1024];
  size_t n = 0;
  size_t i = 0;
  size_t k = 0;
  size_t t = 0;

  if (points == NULL || out_points == NULL || out_n == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: null pointer in convex_hull");
  }
  if (n_points == 0) {
    *out_n = 0;
    return MYLIB_OK;
  }
  if (n_points > 512) {
    return set_error(MYLIB_INVALID_ARGUMENT,
                     "INVALID_ARGUMENT: mock hull supports up to 512 points");
  }

  memcpy(buf, points, n_points * sizeof(mylib_point2d));
  qsort(buf, n_points, sizeof(mylib_point2d), point_cmp);

  for (i = 0; i < n_points; i++) {
    if (i == 0 || buf[i].x != buf[i - 1].x || buf[i].y != buf[i - 1].y) {
      buf[n++] = buf[i];
    }
  }

  if (n == 1) {
    if (out_capacity < 1) {
      return set_error(MYLIB_BUFFER_TOO_SMALL, "BUFFER_TOO_SMALL: hull output too small");
    }
    out_points[0] = buf[0];
    *out_n = 1;
    return MYLIB_OK;
  }

  for (i = 0; i < n; i++) {
    while (k >= 2 && cross(hull[k - 2], hull[k - 1], buf[i]) <= 0.0) {
      k--;
    }
    hull[k++] = buf[i];
  }

  t = k + 1;
  for (i = n - 1; i-- > 0;) {
    while (k >= t && cross(hull[k - 2], hull[k - 1], buf[i]) <= 0.0) {
      k--;
    }
    hull[k++] = buf[i];
  }

  k--;
  if (out_capacity < k) {
    return set_error(MYLIB_BUFFER_TOO_SMALL, "BUFFER_TOO_SMALL: hull output too small");
  }
  memcpy(out_points, hull, k * sizeof(mylib_point2d));
  *out_n = k;
  return MYLIB_OK;
}

mylib_status mylib_polygon_contains(const mylib_point2d *polygon, size_t polygon_points,
                                   mylib_point2d p, bool *out_contains) {
  size_t i = 0;
  size_t unique = 0;
  bool inside = false;

  if (polygon == NULL || out_contains == NULL) {
    return set_error(MYLIB_INVALID_ARGUMENT,
                     "INVALID_ARGUMENT: null pointer in polygon_contains");
  }

  for (i = 0; i < polygon_points; i++) {
    size_t j = 0;
    bool seen = false;
    for (j = 0; j < i; j++) {
      if (polygon[i].x == polygon[j].x && polygon[i].y == polygon[j].y) {
        seen = true;
        break;
      }
    }
    if (!seen) unique++;
  }
  if (unique < 3) {
    return set_error(MYLIB_INVALID_ARGUMENT,
                     "INVALID_ARGUMENT: polygon must have >=3 unique points");
  }

  for (i = 0; i < polygon_points; i++) {
    size_t j = (i + 1) % polygon_points;
    mylib_point2d a = polygon[i];
    mylib_point2d b = polygon[j];
    bool intersects = ((a.y > p.y) != (b.y > p.y)) &&
                      (p.x < (b.x - a.x) * (p.y - a.y) / ((b.y - a.y) + 1e-15) + a.x);
    if (intersects) inside = !inside;
  }

  *out_contains = inside;
  return MYLIB_OK;
}
