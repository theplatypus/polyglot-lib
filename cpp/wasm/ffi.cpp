#include <cstddef>

#include "mylib.h"

extern "C" {

mylib_status mylib_wasm_polygon_contains(const mylib_point2d* polygon, std::size_t polygon_points,
                                         double point_x, double point_y,
                                         bool* out_contains) {
  mylib_point2d p{point_x, point_y};
  return mylib_polygon_contains(polygon, polygon_points, p, out_contains);
}

}
