cmake_minimum_required(VERSION 3.20)
project(mylib_cpp_example LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MYLIB_USE_MOCK "Build with local mock C ABI implementation" ON)
set(MYLIB_ROOT "" CACHE PATH "Root directory of extracted mylib release assets")
set(MYLIB_INCLUDE_DIR "" CACHE PATH "Directory containing mylib.h")
set(MYLIB_LIB "" CACHE FILEPATH "Full path to mylib native library")

set(MYLIB_SOURCE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

add_executable(mylib_cpp_example src/main.cpp)

if(MYLIB_USE_MOCK)
  target_include_directories(mylib_cpp_example PRIVATE "${MYLIB_SOURCE_INCLUDE_DIR}")

  add_library(mylib_mock STATIC src/mock_mylib.c)
  target_include_directories(mylib_mock PUBLIC "${MYLIB_SOURCE_INCLUDE_DIR}")
  target_link_libraries(mylib_cpp_example PRIVATE mylib_mock)
else()
  if(MYLIB_INCLUDE_DIR)
    set(_mylib_include_dir "${MYLIB_INCLUDE_DIR}")
  elseif(MYLIB_ROOT)
    set(_mylib_include_dir "${MYLIB_ROOT}/include")
  else()
    set(_mylib_include_dir "${MYLIB_SOURCE_INCLUDE_DIR}")
  endif()

  if(NOT EXISTS "${_mylib_include_dir}/mylib.h")
    message(FATAL_ERROR
      "Could not find mylib.h in '${_mylib_include_dir}'. "
      "Set MYLIB_INCLUDE_DIR or MYLIB_ROOT to extracted headers.")
  endif()

  if(MYLIB_LIB)
    set(_mylib_lib_path "${MYLIB_LIB}")
  elseif(MYLIB_ROOT)
    find_library(_mylib_lib_path
      NAMES mylib libmylib
      PATHS
        "${MYLIB_ROOT}"
        "${MYLIB_ROOT}/lib"
        "${MYLIB_ROOT}/lib64"
      NO_DEFAULT_PATH
    )
  endif()

  if(NOT _mylib_lib_path)
    message(FATAL_ERROR
      "Could not resolve mylib native library. "
      "Set MYLIB_LIB explicitly or set MYLIB_ROOT to extracted release assets.")
  endif()

  message(STATUS "Using mylib include dir: ${_mylib_include_dir}")
  message(STATUS "Using mylib native lib: ${_mylib_lib_path}")

  target_include_directories(mylib_cpp_example PRIVATE "${_mylib_include_dir}")
  target_link_libraries(mylib_cpp_example PRIVATE "${_mylib_lib_path}")
endif()
