cmake_minimum_required(VERSION 3.20)
project(mylib_cpp_example LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MYLIB_USE_MOCK "Build with local mock C ABI implementation" ON)

add_executable(mylib_cpp_example src/main.cpp)
target_include_directories(mylib_cpp_example PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

if(MYLIB_USE_MOCK)
  add_library(mylib_mock STATIC src/mock_mylib.c)
  target_include_directories(mylib_mock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
  target_link_libraries(mylib_cpp_example PRIVATE mylib_mock)
else()
  if(NOT DEFINED MYLIB_LIB)
    message(FATAL_ERROR "Set MYLIB_LIB to real library path when MYLIB_USE_MOCK=OFF")
  endif()
  target_link_libraries(mylib_cpp_example PRIVATE ${MYLIB_LIB})
endif()
