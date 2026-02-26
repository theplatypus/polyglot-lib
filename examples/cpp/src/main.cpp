#include <array>
#include <iostream>
#include <vector>

#include "mylib.h"

static void require_ok(mylib_status status, const char* context) {
  if (status != MYLIB_OK) {
    std::cerr << context << " failed: " << mylib_last_error_message() << "\n";
    std::exit(1);
  }
}

int main() {
  double scalar = 0.0;
  require_ok(mylib_add(2.0, 3.0, &scalar), "add");
  std::cout << "add: " << scalar << "\n";

  require_ok(mylib_div(9.0, 3.0, &scalar), "div");
  std::cout << "div: " << scalar << "\n";

  std::vector<double> x{1.0, 2.0, 3.0};
  std::vector<double> y{0.5, 1.5, 2.5};
  std::vector<double> out(3, 0.0);

  require_ok(mylib_axpy(2.0, x.data(), x.size(), y.data(), y.size(), out.data(), out.size()),
             "axpy");
  std::cout << "axpy: [" << out[0] << ", " << out[1] << ", " << out[2] << "]\n";

  require_ok(mylib_dot(x.data(), x.size(), y.data(), y.size(), &scalar), "dot");
  std::cout << "dot: " << scalar << "\n";

  std::vector<double> a{1.0, 2.0, 3.0, 4.0};
  std::vector<double> b{5.0, 6.0, 7.0, 8.0};
  std::vector<double> c(4, 0.0);
  require_ok(mylib_matmul(a.data(), 2, 2, b.data(), 2, 2, c.data(), 2, 2), "matmul");
  std::cout << "matmul: [[" << c[0] << ", " << c[1] << "], [" << c[2] << ", " << c[3]
            << "]]\n";

  std::vector<double> axis0(2, 0.0);
  require_ok(mylib_sum_axis0(a.data(), 2, 2, axis0.data(), axis0.size()), "sum_axis0");
  std::cout << "sum_axis0: [" << axis0[0] << ", " << axis0[1] << "]\n";

  std::vector<mylib_point2d> points{{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 1}};
  double xmin = 0.0, ymin = 0.0, xmax = 0.0, ymax = 0.0;
  require_ok(mylib_bounding_box(points.data(), points.size(), &xmin, &ymin, &xmax, &ymax),
             "bounding_box");
  std::cout << "bounding_box: (" << xmin << ", " << ymin << ", " << xmax << ", " << ymax
            << ")\n";

  std::vector<mylib_point2d> hull_out(16);
  size_t hull_n = 0;
  require_ok(mylib_convex_hull(points.data(), points.size(), hull_out.data(), hull_out.size(),
                               &hull_n),
             "convex_hull_into");
  std::cout << "convex_hull_into count: " << hull_n << "\n";

  bool contains = false;
  std::vector<mylib_point2d> polygon{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
  require_ok(mylib_polygon_contains(polygon.data(), polygon.size(), {1, 1}, &contains),
             "polygon_contains");
  std::cout << "polygon_contains (1,1): " << (contains ? "true" : "false") << "\n";

  // Error demonstrations.
  if (mylib_div(1.0, 0.0, &scalar) != MYLIB_OK) {
    std::cout << "expected div error: " << mylib_last_error_message() << "\n";
  }
  if (mylib_axpy(1.0, x.data(), x.size(), y.data(), y.size() - 1, out.data(), out.size()) !=
      MYLIB_OK) {
    std::cout << "expected shape error: " << mylib_last_error_message() << "\n";
  }

  return 0;
}
