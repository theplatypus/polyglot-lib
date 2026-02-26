#include "mylib_core.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mylib {
namespace {

inline Result<bool> check_ptr(const void* ptr, const char* name) {
  if (ptr == nullptr) {
    return Result<bool>::failure(StatusCode::INVALID_ARGUMENT,
                                 std::string(name) + " pointer is null");
  }
  return Result<bool>::success(true);
}

inline double cross(const Point2& o, const Point2& a, const Point2& b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

inline std::vector<Point2> unique_sorted(std::vector<Point2> points) {
  std::sort(points.begin(), points.end(), [](const Point2& a, const Point2& b) {
    if (a.x < b.x) return true;
    if (a.x > b.x) return false;
    return a.y < b.y;
  });
  points.erase(std::unique(points.begin(), points.end(), [](const Point2& a, const Point2& b) {
                 return a.x == b.x && a.y == b.y;
               }),
               points.end());
  return points;
}

inline Result<bool> check_matrix(const MatRef& m, const char* name) {
  if (m.data == nullptr && (m.rows != 0 || m.cols != 0)) {
    return Result<bool>::failure(StatusCode::INVALID_ARGUMENT,
                                 std::string(name) + " data is null");
  }
  if (m.rows > 0 && m.cols > (SIZE_MAX / m.rows)) {
    return Result<bool>::failure(StatusCode::INVALID_ARGUMENT,
                                 std::string(name) + " shape overflows");
  }
  return Result<bool>::success(true);
}

}  // namespace

double add(double a, double b) { return a + b; }

Result<double> div(double a, double b) {
  if (b == 0.0) {
    return Result<double>::failure(StatusCode::DIV_BY_ZERO,
                                   "DIV_BY_ZERO: denominator must be non-zero");
  }
  return Result<double>::success(a / b);
}

Result<std::vector<double>> axpy(double a, const std::vector<double>& x,
                                 const std::vector<double>& y) {
  if (x.size() != y.size()) {
    return Result<std::vector<double>>::failure(
        StatusCode::SHAPE_MISMATCH, "SHAPE_MISMATCH: x and y lengths must match");
  }
  std::vector<double> out(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) {
    out[i] = a * x[i] + y[i];
  }
  return Result<std::vector<double>>::success(std::move(out));
}

Result<bool> axpy_into(double a, const double* x, std::size_t x_len, const double* y,
                       std::size_t y_len, double* out, std::size_t out_len) {
  auto x_ok = check_ptr(x, "x");
  if (!x_ok.ok) return x_ok;
  auto y_ok = check_ptr(y, "y");
  if (!y_ok.ok) return y_ok;
  auto out_ok = check_ptr(out, "out");
  if (!out_ok.ok) return out_ok;
  if (x_len != y_len || x_len != out_len) {
    return Result<bool>::failure(StatusCode::SHAPE_MISMATCH,
                                 "SHAPE_MISMATCH: x/y/out lengths must match");
  }
  for (std::size_t i = 0; i < x_len; ++i) {
    out[i] = a * x[i] + y[i];
  }
  return Result<bool>::success(true);
}

Result<double> dot(const double* x, std::size_t x_len, const double* y, std::size_t y_len) {
  auto x_ok = check_ptr(x, "x");
  if (!x_ok.ok) return Result<double>::failure(x_ok.error.code, x_ok.error.message);
  auto y_ok = check_ptr(y, "y");
  if (!y_ok.ok) return Result<double>::failure(y_ok.error.code, y_ok.error.message);
  if (x_len != y_len) {
    return Result<double>::failure(StatusCode::SHAPE_MISMATCH,
                                   "SHAPE_MISMATCH: x and y lengths must match");
  }
  if (x_len == 0) {
    return Result<double>::failure(StatusCode::INVALID_ARGUMENT,
                                   "INVALID_ARGUMENT: dot requires non-empty vectors");
  }
  double acc = 0.0;
  for (std::size_t i = 0; i < x_len; ++i) {
    acc += x[i] * y[i];
  }
  return Result<double>::success(acc);
}

Result<std::vector<double>> matmul(const MatRef& a, const MatRef& b) {
  auto a_ok = check_matrix(a, "a");
  if (!a_ok.ok) return Result<std::vector<double>>::failure(a_ok.error.code, a_ok.error.message);
  auto b_ok = check_matrix(b, "b");
  if (!b_ok.ok) return Result<std::vector<double>>::failure(b_ok.error.code, b_ok.error.message);
  if (a.cols != b.rows) {
    return Result<std::vector<double>>::failure(StatusCode::SHAPE_MISMATCH,
                                                "SHAPE_MISMATCH: a.cols must equal b.rows");
  }
  std::vector<double> out(a.rows * b.cols, 0.0);
  auto r = matmul_into(a, b, MatMutRef{out.data(), a.rows, b.cols});
  if (!r.ok) {
    return Result<std::vector<double>>::failure(r.error.code, r.error.message);
  }
  return Result<std::vector<double>>::success(std::move(out));
}

Result<bool> matmul_into(const MatRef& a, const MatRef& b, const MatMutRef& out) {
  auto a_ok = check_matrix(a, "a");
  if (!a_ok.ok) return a_ok;
  auto b_ok = check_matrix(b, "b");
  if (!b_ok.ok) return b_ok;
  if (out.data == nullptr) {
    return Result<bool>::failure(StatusCode::INVALID_ARGUMENT, "INVALID_ARGUMENT: out is null");
  }
  if (a.cols != b.rows) {
    return Result<bool>::failure(StatusCode::SHAPE_MISMATCH,
                                 "SHAPE_MISMATCH: a.cols must equal b.rows");
  }
  if (out.rows != a.rows || out.cols != b.cols) {
    return Result<bool>::failure(StatusCode::SHAPE_MISMATCH,
                                 "SHAPE_MISMATCH: out must be (a.rows, b.cols)");
  }

  for (std::size_t r = 0; r < out.rows; ++r) {
    for (std::size_t c = 0; c < out.cols; ++c) {
      double sum = 0.0;
      for (std::size_t k = 0; k < a.cols; ++k) {
        sum += a.data[r * a.cols + k] * b.data[k * b.cols + c];
      }
      out.data[r * out.cols + c] = sum;
    }
  }
  return Result<bool>::success(true);
}

Result<std::vector<double>> sum_axis0(const MatRef& x) {
  auto x_ok = check_matrix(x, "x");
  if (!x_ok.ok) return Result<std::vector<double>>::failure(x_ok.error.code, x_ok.error.message);

  std::vector<double> out(x.cols, 0.0);
  auto r = sum_axis0_into(x, out.data(), out.size());
  if (!r.ok) {
    return Result<std::vector<double>>::failure(r.error.code, r.error.message);
  }
  return Result<std::vector<double>>::success(std::move(out));
}

Result<bool> sum_axis0_into(const MatRef& x, double* out, std::size_t out_len) {
  auto x_ok = check_matrix(x, "x");
  if (!x_ok.ok) return x_ok;
  auto out_ok = check_ptr(out, "out");
  if (!out_ok.ok) return out_ok;
  if (out_len != x.cols) {
    return Result<bool>::failure(StatusCode::SHAPE_MISMATCH,
                                 "SHAPE_MISMATCH: out length must equal cols");
  }
  for (std::size_t c = 0; c < x.cols; ++c) {
    out[c] = 0.0;
  }
  for (std::size_t r = 0; r < x.rows; ++r) {
    for (std::size_t c = 0; c < x.cols; ++c) {
      out[c] += x.data[r * x.cols + c];
    }
  }
  return Result<bool>::success(true);
}

Result<std::vector<double>> bounding_box(const std::vector<Point2>& points) {
  if (points.empty()) {
    return Result<std::vector<double>>::failure(StatusCode::INVALID_ARGUMENT,
                                                "INVALID_ARGUMENT: points cannot be empty");
  }
  double xmin = points[0].x;
  double ymin = points[0].y;
  double xmax = points[0].x;
  double ymax = points[0].y;

  for (const auto& p : points) {
    xmin = std::min(xmin, p.x);
    ymin = std::min(ymin, p.y);
    xmax = std::max(xmax, p.x);
    ymax = std::max(ymax, p.y);
  }

  return Result<std::vector<double>>::success({xmin, ymin, xmax, ymax});
}

std::vector<Point2> convex_hull(const std::vector<Point2>& points) {
  auto pts = unique_sorted(points);
  if (pts.size() <= 1) {
    return pts;
  }

  std::vector<Point2> lower;
  for (const auto& p : pts) {
    while (lower.size() >= 2 &&
           cross(lower[lower.size() - 2], lower[lower.size() - 1], p) <= 0.0) {
      lower.pop_back();
    }
    lower.push_back(p);
  }

  std::vector<Point2> upper;
  for (std::size_t i = pts.size(); i-- > 0;) {
    const auto& p = pts[i];
    while (upper.size() >= 2 &&
           cross(upper[upper.size() - 2], upper[upper.size() - 1], p) <= 0.0) {
      upper.pop_back();
    }
    upper.push_back(p);
  }

  lower.pop_back();
  upper.pop_back();
  lower.insert(lower.end(), upper.begin(), upper.end());
  return lower;
}

Result<std::size_t> convex_hull_into(const std::vector<Point2>& points, Point2* out,
                                     std::size_t out_capacity) {
  auto out_ok = check_ptr(out, "out");
  if (!out_ok.ok) {
    return Result<std::size_t>::failure(out_ok.error.code, out_ok.error.message);
  }
  auto hull = convex_hull(points);
  if (out_capacity < hull.size()) {
    return Result<std::size_t>::failure(StatusCode::BUFFER_TOO_SMALL,
                                        "BUFFER_TOO_SMALL: out buffer too small for hull");
  }
  for (std::size_t i = 0; i < hull.size(); ++i) {
    out[i] = hull[i];
  }
  return Result<std::size_t>::success(hull.size());
}

Result<bool> polygon_contains(const std::vector<Point2>& polygon, const Point2& point) {
  auto uniq = unique_sorted(polygon);
  if (uniq.size() < 3) {
    return Result<bool>::failure(StatusCode::INVALID_ARGUMENT,
                                 "INVALID_ARGUMENT: polygon must have >=3 unique points");
  }
  bool inside = false;
  for (std::size_t i = 0; i < polygon.size(); ++i) {
    std::size_t j = (i + 1) % polygon.size();
    const Point2& a = polygon[i];
    const Point2& b = polygon[j];
    bool intersects = ((a.y > point.y) != (b.y > point.y)) &&
                      (point.x < (b.x - a.x) * (point.y - a.y) / ((b.y - a.y) + 1e-15) + a.x);
    if (intersects) {
      inside = !inside;
    }
  }
  return Result<bool>::success(inside);
}

}  // namespace mylib
