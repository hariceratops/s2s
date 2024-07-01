project(struct_cast)
cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAS_RELEASE "-O3")

# Set the paths
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SINGLE_HEADER_DIR "${CMAKE_SOURCE_DIR}/single_header")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")

# Define the input and output for quom
set(INPUT_HEADER "${INCLUDE_DIR}/struct_cast.hpp")
set(OUTPUT_HEADER "${SINGLE_HEADER_DIR}/struct_cast.hpp")

# Custom command to run quom
add_custom_command(
    OUTPUT ${OUTPUT_HEADER}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SINGLE_HEADER_DIR}
    COMMAND python -m quom ${INPUT_HEADER} ${OUTPUT_HEADER}
    DEPENDS ${INPUT_HEADER}
    COMMENT "Generating single header using quom"
    VERBATIM
)

# Custom target that depends on the generated header
add_custom_target(generate_single_header ALL DEPENDS ${OUTPUT_HEADER})

# Add the test executable
add_executable(test_struct_cast ${TEST_DIR}/test_struct_cast.cpp)

set_target_properties(
  test_struct_cast
  PROPERTIES
  CXX_STANDARD 20
)

