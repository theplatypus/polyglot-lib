#ifndef CPP_MYLIB_CORE_HPP
#define CPP_MYLIB_CORE_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace mylib {

enum class StatusCode {
  OK = 0,
  INVALID_ARGUMENT = 1,
  SHAPE_MISMATCH = 2,
  DIV_BY_ZERO = 3,
  BUFFER_TOO_SMALL = 4,
  INTERNAL_ERROR = 5,
};

struct Error {
  StatusCode code;
  std::string message;
};

template <typename T>
struct Result {
  T value;
  Error error{StatusCode::OK, "OK"};
  bool ok = true;

  static Result success(T v) { return Result{std::move(v), {StatusCode::OK, "OK"}, true}; }
  static Result failure(StatusCode code, std::string message) {
    return Result{T{}, {code, std::move(message)}, false};
  }
};

struct Point2 {
  double x;
  double y;
};

struct MatRef {
  const double* data;
  std::size_t rows;
  std::size_t cols;
};

struct MatMutRef {
  double* data;
  std::size_t rows;
  std::size_t cols;
};

/** Add two scalar values. */
double add(double a, double b);
/** Divide two scalar values. Returns DIV_BY_ZERO when b == 0. */
Result<double> div(double a, double b);

Result<std::vector<double>> axpy(double a, const std::vector<double>& x,
                                 const std::vector<double>& y);
Result<bool> axpy_into(double a, const double* x, std::size_t x_len, const double* y,
                       std::size_t y_len, double* out, std::size_t out_len);
Result<double> dot(const double* x, std::size_t x_len, const double* y, std::size_t y_len);

Result<std::vector<double>> matmul(const MatRef& a, const MatRef& b);
Result<bool> matmul_into(const MatRef& a, const MatRef& b, const MatMutRef& out);
Result<std::vector<double>> sum_axis0(const MatRef& x);
Result<bool> sum_axis0_into(const MatRef& x, double* out, std::size_t out_len);

Result<std::vector<double>> bounding_box(const std::vector<Point2>& points);
std::vector<Point2> convex_hull(const std::vector<Point2>& points);
Result<std::size_t> convex_hull_into(const std::vector<Point2>& points, Point2* out,
                                     std::size_t out_capacity);
Result<bool> polygon_contains(const std::vector<Point2>& polygon, const Point2& point);

}  // namespace mylib

#endif
