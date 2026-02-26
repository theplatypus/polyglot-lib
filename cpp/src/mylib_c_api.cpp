#include "mylib.h"

#include <string>
#include <vector>

#include "mylib_core.hpp"

namespace {
thread_local std::string g_last_error = "OK";

mylib_status set_error(mylib_status status, const std::string& msg) {
  g_last_error = msg;
  return status;
}

mylib_status from_status(mylib::StatusCode code) {
  switch (code) {
    case mylib::StatusCode::OK:
      return MYLIB_OK;
    case mylib::StatusCode::INVALID_ARGUMENT:
      return MYLIB_INVALID_ARGUMENT;
    case mylib::StatusCode::SHAPE_MISMATCH:
      return MYLIB_SHAPE_MISMATCH;
    case mylib::StatusCode::DIV_BY_ZERO:
      return MYLIB_DIV_BY_ZERO;
    case mylib::StatusCode::BUFFER_TOO_SMALL:
      return MYLIB_BUFFER_TOO_SMALL;
    case mylib::StatusCode::INTERNAL_ERROR:
      return MYLIB_INTERNAL_ERROR;
  }
  return MYLIB_INTERNAL_ERROR;
}

template <typename F>
mylib_status guard(F fn) {
  try {
    return fn();
  } catch (const std::exception& e) {
    return set_error(MYLIB_INTERNAL_ERROR, std::string("INTERNAL_ERROR: ") + e.what());
  } catch (...) {
    return set_error(MYLIB_INTERNAL_ERROR, "INTERNAL_ERROR: unknown exception");
  }
}

std::vector<mylib::Point2> to_points(const mylib_point2d* points, std::size_t n) {
  std::vector<mylib::Point2> out;
  out.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    out.push_back(mylib::Point2{points[i].x, points[i].y});
  }
  return out;
}

}  // namespace

extern "C" {

const char* mylib_last_error_message(void) { return g_last_error.c_str(); }

mylib_status mylib_add(double a, double b, double* out) {
  return guard([&]() -> mylib_status {
    if (out == nullptr) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out is null");
    }
    *out = mylib::add(a, b);
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_div(double a, double b, double* out) {
  return guard([&]() -> mylib_status {
    if (out == nullptr) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out is null");
    }
    auto r = mylib::div(a, b);
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    *out = r.value;
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_axpy(double a, const double* x, size_t x_len, const double* y, size_t y_len,
                        double* out, size_t out_len) {
  return guard([&]() -> mylib_status {
    auto r = mylib::axpy_into(a, x, x_len, y, y_len, out, out_len);
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_dot(const double* x, size_t x_len, const double* y, size_t y_len,
                       double* out) {
  return guard([&]() -> mylib_status {
    if (out == nullptr) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out is null");
    }
    auto r = mylib::dot(x, x_len, y, y_len);
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    *out = r.value;
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_matmul(const double* a, size_t a_rows, size_t a_cols, const double* b,
                          size_t b_rows, size_t b_cols, double* out, size_t out_rows,
                          size_t out_cols) {
  return guard([&]() -> mylib_status {
    auto r = mylib::matmul_into(mylib::MatRef{a, a_rows, a_cols}, mylib::MatRef{b, b_rows, b_cols},
                                mylib::MatMutRef{out, out_rows, out_cols});
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_sum_axis0(const double* x, size_t rows, size_t cols, double* out,
                             size_t out_len) {
  return guard([&]() -> mylib_status {
    auto r = mylib::sum_axis0_into(mylib::MatRef{x, rows, cols}, out, out_len);
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_bounding_box(const mylib_point2d* points, size_t n_points, double* xmin,
                                double* ymin, double* xmax, double* ymax) {
  return guard([&]() -> mylib_status {
    if (xmin == nullptr || ymin == nullptr || xmax == nullptr || ymax == nullptr) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: output pointer is null");
    }
    if (points == nullptr && n_points != 0) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: points is null");
    }
    auto r = mylib::bounding_box(to_points(points, n_points));
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    *xmin = r.value[0];
    *ymin = r.value[1];
    *xmax = r.value[2];
    *ymax = r.value[3];
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_convex_hull(const mylib_point2d* points, size_t n_points,
                               mylib_point2d* out_points, size_t out_capacity, size_t* out_n) {
  return guard([&]() -> mylib_status {
    if (out_n == nullptr) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out_n is null");
    }
    if (points == nullptr && n_points != 0) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: points is null");
    }
    if (out_points == nullptr && out_capacity != 0) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out_points is null");
    }

    std::vector<mylib::Point2> p = to_points(points, n_points);
    std::vector<mylib::Point2> tmp(out_capacity, mylib::Point2{0.0, 0.0});
    auto r = mylib::convex_hull_into(p, tmp.data(), out_capacity);
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    *out_n = r.value;
    for (std::size_t i = 0; i < r.value; ++i) {
      out_points[i].x = tmp[i].x;
      out_points[i].y = tmp[i].y;
    }
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

mylib_status mylib_polygon_contains(const mylib_point2d* polygon, size_t polygon_points,
                                    mylib_point2d p, bool* out_contains) {
  return guard([&]() -> mylib_status {
    if (out_contains == nullptr) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: out_contains is null");
    }
    if (polygon == nullptr && polygon_points != 0) {
      return set_error(MYLIB_INVALID_ARGUMENT, "INVALID_ARGUMENT: polygon is null");
    }
    auto r = mylib::polygon_contains(to_points(polygon, polygon_points), mylib::Point2{p.x, p.y});
    if (!r.ok) {
      return set_error(from_status(r.error.code), r.error.message);
    }
    *out_contains = r.value;
    g_last_error = "OK";
    return MYLIB_OK;
  });
}

}  // extern "C"
