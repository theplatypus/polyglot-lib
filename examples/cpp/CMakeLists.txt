cmake_minimum_required(VERSION 3.20)
project(mylib_cpp_example LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MYLIB_USE_MOCK "Build with local mock C ABI implementation" OFF)

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
    set(MYLIB_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/build/libmylib_c_api.a)
  endif()
  if(NOT DEFINED MYLIB_CORE_LIB)
    set(MYLIB_CORE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/build/libmylib_core.a)
  endif()
  if(NOT EXISTS ${MYLIB_LIB})
    message(FATAL_ERROR "Set MYLIB_LIB to built C ABI static/shared library path")
  endif()
  if(EXISTS ${MYLIB_CORE_LIB})
    target_link_libraries(mylib_cpp_example PRIVATE ${MYLIB_LIB} ${MYLIB_CORE_LIB})
  else()
    target_link_libraries(mylib_cpp_example PRIVATE ${MYLIB_LIB})
  endif()
endif()
