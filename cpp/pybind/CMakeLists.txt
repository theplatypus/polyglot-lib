cmake_minimum_required(VERSION 3.20)
project(mylib_cpp_py LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(mylib_cpp module.cpp ../src/mylib_core.cpp)
target_include_directories(mylib_cpp PRIVATE ../include)

install(TARGETS mylib_cpp LIBRARY DESTINATION . RUNTIME DESTINATION .)
