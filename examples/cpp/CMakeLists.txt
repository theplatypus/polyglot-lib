cmake_minimum_required(VERSION 3.20)
project(mylib_cpp_example LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MYLIB_USE_MOCK "Build with local mock C ABI implementation" OFF)
set(MYLIB_ROOT "" CACHE PATH "Root directory of extracted mylib release assets")
set(MYLIB_INCLUDE_DIR "" CACHE PATH "Directory containing mylib.h")
set(MYLIB_LIB "" CACHE FILEPATH "Full path to C ABI library (mylib_c_api)")
set(MYLIB_CORE_LIB "" CACHE FILEPATH "Optional full path to core library (mylib_core)")

set(MYLIB_SOURCE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
set(MYLIB_SOURCE_C_API_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/build/libmylib_c_api.a")
set(MYLIB_SOURCE_CORE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/build/libmylib_core.a")

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
      NAMES mylib_c_api libmylib_c_api mylib libmylib
      PATHS
        "${MYLIB_ROOT}"
        "${MYLIB_ROOT}/lib"
        "${MYLIB_ROOT}/lib64"
      NO_DEFAULT_PATH
    )
    if(NOT _mylib_lib_path)
      foreach(_candidate
        "${MYLIB_ROOT}/libmylib_c_api.a"
        "${MYLIB_ROOT}/libmylib_c_api.so"
        "${MYLIB_ROOT}/libmylib_c_api.dylib"
        "${MYLIB_ROOT}/mylib_c_api.lib"
        "${MYLIB_ROOT}/mylib_c_api.dll")
        if(EXISTS "${_candidate}")
          set(_mylib_lib_path "${_candidate}")
          break()
        endif()
      endforeach()
    endif()
  elseif(EXISTS "${MYLIB_SOURCE_C_API_LIB}")
    set(_mylib_lib_path "${MYLIB_SOURCE_C_API_LIB}")
  endif()

  if(NOT _mylib_lib_path)
    message(FATAL_ERROR
      "Could not resolve C ABI library. "
      "Set MYLIB_LIB explicitly or set MYLIB_ROOT to extracted release assets.")
  endif()

  set(_mylib_core_lib_path "")
  if(MYLIB_CORE_LIB)
    set(_mylib_core_lib_path "${MYLIB_CORE_LIB}")
  elseif(MYLIB_ROOT)
    find_library(_mylib_core_lib_path
      NAMES mylib_core libmylib_core
      PATHS
        "${MYLIB_ROOT}"
        "${MYLIB_ROOT}/lib"
        "${MYLIB_ROOT}/lib64"
      NO_DEFAULT_PATH
    )
    if(NOT _mylib_core_lib_path)
      foreach(_candidate
        "${MYLIB_ROOT}/libmylib_core.a"
        "${MYLIB_ROOT}/libmylib_core.so"
        "${MYLIB_ROOT}/libmylib_core.dylib"
        "${MYLIB_ROOT}/mylib_core.lib"
        "${MYLIB_ROOT}/mylib_core.dll")
        if(EXISTS "${_candidate}")
          set(_mylib_core_lib_path "${_candidate}")
          break()
        endif()
      endforeach()
    endif()
  elseif(EXISTS "${MYLIB_SOURCE_CORE_LIB}")
    set(_mylib_core_lib_path "${MYLIB_SOURCE_CORE_LIB}")
  endif()

  message(STATUS "Using mylib include dir: ${_mylib_include_dir}")
  message(STATUS "Using mylib C ABI lib: ${_mylib_lib_path}")
  if(_mylib_core_lib_path)
    message(STATUS "Using mylib core lib: ${_mylib_core_lib_path}")
  endif()

  target_include_directories(mylib_cpp_example PRIVATE "${_mylib_include_dir}")
  target_link_libraries(mylib_cpp_example PRIVATE "${_mylib_lib_path}")
  if(_mylib_core_lib_path)
    target_link_libraries(mylib_cpp_example PRIVATE "${_mylib_core_lib_path}")
  endif()
endif()
