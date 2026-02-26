#ifndef MYLIB_H
#define MYLIB_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mylib_status {
  MYLIB_OK = 0,
  MYLIB_INVALID_ARGUMENT = 1,
  MYLIB_SHAPE_MISMATCH = 2,
  MYLIB_DIV_BY_ZERO = 3,
  MYLIB_BUFFER_TOO_SMALL = 4,
  MYLIB_INTERNAL_ERROR = 5
} mylib_status;

typedef struct mylib_point2d {
  double x;
  double y;
} mylib_point2d;

/* Error utilities */
const char *mylib_last_error_message(void);

/* Scalars */
mylib_status mylib_add(double a, double b, double *out);
mylib_status mylib_div(double a, double b, double *out);

/* Vectors (1D) */
mylib_status mylib_axpy(double a, const double *x, size_t x_len, const double *y,
                       size_t y_len, double *out, size_t out_len);
mylib_status mylib_dot(const double *x, size_t x_len, const double *y, size_t y_len,
                      double *out);

/* Tensors (2D row-major contiguous) */
mylib_status mylib_matmul(const double *a, size_t a_rows, size_t a_cols,
                         const double *b, size_t b_rows, size_t b_cols,
                         double *out, size_t out_rows, size_t out_cols);
mylib_status mylib_sum_axis0(const double *x, size_t rows, size_t cols, double *out,
                            size_t out_len);

/* Geometry */
mylib_status mylib_bounding_box(const mylib_point2d *points, size_t n_points,
                               double *xmin, double *ymin, double *xmax,
                               double *ymax);
mylib_status mylib_convex_hull(const mylib_point2d *points, size_t n_points,
                              mylib_point2d *out_points, size_t out_capacity,
                              size_t *out_n);
mylib_status mylib_polygon_contains(const mylib_point2d *polygon,
                                   size_t polygon_points, mylib_point2d p,
                                   bool *out_contains);

#ifdef __cplusplus
}
#endif

#endif /* MYLIB_H */
