project(struct_cast)
cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAS_RELEASE "-O3")

add_executable(struct_cast test/test_struct_cast.cpp)

set_target_properties(
  struct_cast
  PROPERTIES
  CXX_STANDARD 20
)

