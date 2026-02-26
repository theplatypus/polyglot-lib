#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <cstring>
#include <stdexcept>
#include <vector>

#include "mylib_core.hpp"

namespace py = pybind11;

namespace {

[[noreturn]] void throw_py(const mylib::Error& err) {
  switch (err.code) {
    case mylib::StatusCode::DIV_BY_ZERO:
      PyErr_SetString(PyExc_ZeroDivisionError, err.message.c_str());
      throw py::error_already_set();
    case mylib::StatusCode::INVALID_ARGUMENT:
    case mylib::StatusCode::SHAPE_MISMATCH:
      throw py::value_error(err.message);
    case mylib::StatusCode::BUFFER_TOO_SMALL:
    case mylib::StatusCode::INTERNAL_ERROR:
    case mylib::StatusCode::OK:
      throw std::runtime_error(err.message);
  }
  throw std::runtime_error(err.message);
}

std::vector<mylib::Point2> points_from_array(const py::array_t<double, py::array::c_style>& arr) {
  if (arr.ndim() != 2 || arr.shape(1) != 2) {
    throw py::value_error("points must have shape (n,2)");
  }
  std::vector<mylib::Point2> out;
  out.reserve(static_cast<std::size_t>(arr.shape(0)));
  auto a = arr.unchecked<2>();
  for (ssize_t i = 0; i < arr.shape(0); ++i) {
    out.push_back(mylib::Point2{a(i, 0), a(i, 1)});
  }
  return out;
}

py::array_t<double> points_to_array(const std::vector<mylib::Point2>& points) {
  py::array_t<double> out(std::vector<py::ssize_t>{static_cast<py::ssize_t>(points.size()), 2});
  auto m = out.mutable_unchecked<2>();
  for (std::size_t i = 0; i < points.size(); ++i) {
    m(static_cast<ssize_t>(i), 0) = points[i].x;
    m(static_cast<ssize_t>(i), 1) = points[i].y;
  }
  return out;
}

}  // namespace

PYBIND11_MODULE(mylib_cpp, m) {
  m.doc() = "pybind11 bindings for cpp-core";

  m.def("add", &mylib::add);

  m.def("div", [](double a, double b) {
    auto r = mylib::div(a, b);
    if (!r.ok) throw_py(r.error);
    return r.value;
  });

  m.def("axpy", [](double a, const py::array_t<double, py::array::c_style>& x,
                    const py::array_t<double, py::array::c_style>& y) {
    if (x.ndim() != 1 || y.ndim() != 1) {
      throw py::value_error("x and y must be 1D");
    }
    std::vector<double> xv(x.size());
    std::vector<double> yv(y.size());
    std::memcpy(xv.data(), x.data(), x.size() * sizeof(double));
    std::memcpy(yv.data(), y.data(), y.size() * sizeof(double));
    auto r = mylib::axpy(a, xv, yv);
    if (!r.ok) throw_py(r.error);
    py::array_t<double> out(static_cast<ssize_t>(r.value.size()));
    std::memcpy(out.mutable_data(), r.value.data(), r.value.size() * sizeof(double));
    return out;
  });

  m.def("axpy_into", [](double a, const py::array_t<double, py::array::c_style>& x,
                         const py::array_t<double, py::array::c_style>& y,
                         py::array_t<double, py::array::c_style>& out) {
    if (x.ndim() != 1 || y.ndim() != 1 || out.ndim() != 1) {
      throw py::value_error("x/y/out must be 1D");
    }
    auto r = mylib::axpy_into(a, x.data(), static_cast<std::size_t>(x.size()), y.data(),
                              static_cast<std::size_t>(y.size()), out.mutable_data(),
                              static_cast<std::size_t>(out.size()));
    if (!r.ok) throw_py(r.error);
  });

  m.def("dot", [](const py::array_t<double, py::array::c_style>& x,
                   const py::array_t<double, py::array::c_style>& y) {
    if (x.ndim() != 1 || y.ndim() != 1) {
      throw py::value_error("x and y must be 1D");
    }
    auto r = mylib::dot(x.data(), static_cast<std::size_t>(x.size()), y.data(),
                        static_cast<std::size_t>(y.size()));
    if (!r.ok) throw_py(r.error);
    return r.value;
  });

  m.def("matmul", [](const py::array_t<double, py::array::c_style>& a,
                      const py::array_t<double, py::array::c_style>& b) {
    if (a.ndim() != 2 || b.ndim() != 2) {
      throw py::value_error("a and b must be 2D");
    }
    auto r = mylib::matmul(mylib::MatRef{a.data(), static_cast<std::size_t>(a.shape(0)),
                                          static_cast<std::size_t>(a.shape(1))},
                           mylib::MatRef{b.data(), static_cast<std::size_t>(b.shape(0)),
                                          static_cast<std::size_t>(b.shape(1))});
    if (!r.ok) throw_py(r.error);
    py::array_t<double> out({a.shape(0), b.shape(1)});
    std::memcpy(out.mutable_data(), r.value.data(), r.value.size() * sizeof(double));
    return out;
  });

  m.def("matmul_into", [](const py::array_t<double, py::array::c_style>& a,
                           const py::array_t<double, py::array::c_style>& b,
                           py::array_t<double, py::array::c_style>& out) {
    if (a.ndim() != 2 || b.ndim() != 2 || out.ndim() != 2) {
      throw py::value_error("a/b/out must be 2D");
    }
    auto r = mylib::matmul_into(
        mylib::MatRef{a.data(), static_cast<std::size_t>(a.shape(0)),
                      static_cast<std::size_t>(a.shape(1))},
        mylib::MatRef{b.data(), static_cast<std::size_t>(b.shape(0)),
                      static_cast<std::size_t>(b.shape(1))},
        mylib::MatMutRef{out.mutable_data(), static_cast<std::size_t>(out.shape(0)),
                         static_cast<std::size_t>(out.shape(1))});
    if (!r.ok) throw_py(r.error);
  });

  m.def("sum_axis0", [](const py::array_t<double, py::array::c_style>& x) {
    if (x.ndim() != 2) {
      throw py::value_error("x must be 2D");
    }
    auto r = mylib::sum_axis0(
        mylib::MatRef{x.data(), static_cast<std::size_t>(x.shape(0)), static_cast<std::size_t>(x.shape(1))});
    if (!r.ok) throw_py(r.error);
    py::array_t<double> out(static_cast<ssize_t>(r.value.size()));
    std::memcpy(out.mutable_data(), r.value.data(), r.value.size() * sizeof(double));
    return out;
  });

  m.def("bounding_box", [](const py::array_t<double, py::array::c_style>& points) {
    auto r = mylib::bounding_box(points_from_array(points));
    if (!r.ok) throw_py(r.error);
    return py::make_tuple(r.value[0], r.value[1], r.value[2], r.value[3]);
  });

  m.def("convex_hull", [](const py::array_t<double, py::array::c_style>& points) {
    return points_to_array(mylib::convex_hull(points_from_array(points)));
  });

  m.def("convex_hull_into", [](const py::array_t<double, py::array::c_style>& points,
                                py::array_t<double, py::array::c_style>& out) {
    if (out.ndim() != 2 || out.shape(1) != 2) {
      throw py::value_error("out must have shape (n,2)");
    }
    std::vector<mylib::Point2> tmp(static_cast<std::size_t>(out.shape(0)));
    auto r = mylib::convex_hull_into(points_from_array(points), tmp.data(), tmp.size());
    if (!r.ok) throw_py(r.error);
    auto outm = out.mutable_unchecked<2>();
    for (std::size_t i = 0; i < r.value; ++i) {
      outm(static_cast<ssize_t>(i), 0) = tmp[i].x;
      outm(static_cast<ssize_t>(i), 1) = tmp[i].y;
    }
    return static_cast<std::size_t>(r.value);
  });

  m.def("polygon_contains", [](const py::array_t<double, py::array::c_style>& polygon,
                                double point_x, double point_y) {
    auto r = mylib::polygon_contains(points_from_array(polygon), mylib::Point2{point_x, point_y});
    if (!r.ok) throw_py(r.error);
    return r.value;
  });
}
