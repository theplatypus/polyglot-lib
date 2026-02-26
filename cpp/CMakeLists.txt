cmake_minimum_required(VERSION 3.20)
project(mylib_cpp_core LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(MYLIB_BUILD_TESTS "Build C++ tests" ON)
option(MYLIB_BUILD_PYTHON "Build pybind11 module" ON)

add_library(mylib_core STATIC src/mylib_core.cpp)
target_include_directories(mylib_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(mylib_c_api STATIC src/mylib_c_api.cpp)
target_include_directories(mylib_c_api
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries(mylib_c_api PUBLIC mylib_core)

if(MYLIB_BUILD_TESTS)
  enable_testing()
  add_executable(mylib_core_tests tests/test_core.cpp)
  target_link_libraries(mylib_core_tests PRIVATE mylib_core)
  add_test(NAME mylib_core_tests COMMAND mylib_core_tests)
endif()

if(MYLIB_BUILD_PYTHON)
  include(FetchContent)
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.12.0
  )
  FetchContent_MakeAvailable(pybind11)

  pybind11_add_module(mylib_cpp pybind/module.cpp)
  target_link_libraries(mylib_cpp PRIVATE mylib_core)
  target_include_directories(mylib_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  set_target_properties(mylib_cpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python
  )
endif()
