#include <cassert>
#include <vector>

#include "mylib_core.hpp"

int main() {
  assert(mylib::add(2.0, 3.0) == 5.0);
  assert(mylib::div(9.0, 3.0).ok);
  assert(!mylib::div(1.0, 0.0).ok);

  std::vector<double> x{1.0, 2.0, 3.0};
  std::vector<double> y{0.5, 1.5, 2.5};
  auto axpy = mylib::axpy(2.0, x, y);
  assert(axpy.ok && axpy.value.size() == 3);
  assert(axpy.value[0] == 2.5);

  auto dot = mylib::dot(x.data(), x.size(), y.data(), y.size());
  assert(dot.ok && dot.value == 11.0);

  double a_data[] = {1.0, 2.0, 3.0, 4.0};
  double b_data[] = {5.0, 6.0, 7.0, 8.0};
  auto mm = mylib::matmul(mylib::MatRef{a_data, 2, 2}, mylib::MatRef{b_data, 2, 2});
  assert(mm.ok && mm.value[0] == 19.0 && mm.value[3] == 50.0);

  auto s0 = mylib::sum_axis0(mylib::MatRef{a_data, 2, 2});
  assert(s0.ok && s0.value[0] == 4.0 && s0.value[1] == 6.0);

  std::vector<mylib::Point2> points{{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 1}};
  auto bb = mylib::bounding_box(points);
  assert(bb.ok && bb.value[0] == 0.0 && bb.value[3] == 2.0);

  auto hull = mylib::convex_hull(points);
  assert(hull.size() == 4);

  auto inside = mylib::polygon_contains({{0, 0}, {2, 0}, {2, 2}, {0, 2}}, {1, 1});
  assert(inside.ok && inside.value);

  return 0;
}
