project(struct_cast)
cmake_minimum_required(VERSION 3.20)

set(SINGLE_HEADER_GENERATOR "AMALGAM" CACHE STRING "Single header generator, choices: AMALGAM, QUOM")

# Add GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Set the paths
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SINGLE_HEADER_DIR "${CMAKE_SOURCE_DIR}/single_header")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
# Define the input and output for quom
set(INPUT_HEADER "${INCLUDE_DIR}/s2s.hpp")
set(OUTPUT_HEADER "${SINGLE_HEADER_DIR}/s2s.hpp")
# Get all header files in the include directory
file(GLOB_RECURSE HEADER_FILES "${INCLUDE_DIR}/*.hpp")

if(SINGLE_HEADER_GENERATOR STREQUAL "AMALGAM")
  add_custom_command(
    OUTPUT ${OUTPUT_HEADER}
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/amalgam.py ${INCLUDE_DIR} ${INPUT_HEADER} ${OUTPUT_HEADER}
    DEPENDS ${HEADER_FILES}
    COMMENT "Generating single header using amalgam"
    VERBATIM
  )
elseif(SINGLE_HEADER_GENERATOR STREQUAL "QUOM")
  # Custom command to run quom
  add_custom_command(
    OUTPUT ${OUTPUT_HEADER}
    COMMAND python -m quom ${INPUT_HEADER} ${OUTPUT_HEADER}
    DEPENDS ${HEADER_FILES}
    COMMENT "Generating single header using quom"
    VERBATIM
  )
else()
  message(FATAL_ERROR "Invalid SINGLE_HEADER_GENERATOR value. Use QUOM or AMALGAM")
endif()

# Custom target that depends on the generated header
add_custom_target(generate_single_header ALL DEPENDS ${OUTPUT_HEADER})

function(add_struct_cast_test name)
  add_executable(${name} ${TEST_DIR}/${name}.cpp)
  target_link_libraries(${name} PRIVATE gtest gtest_main)
  add_test(NAME ${name} COMMAND ${name})
endfunction()


add_struct_cast_test(test_s2s_arr_of_recs_fields)
add_struct_cast_test(test_s2s_computation_from_fields)
add_struct_cast_test(test_s2s_field_validators)
add_struct_cast_test(test_s2s_fixed_buffer_fields)
add_struct_cast_test(test_s2s_magic_fields)
add_struct_cast_test(test_s2s_optional_fields)
add_struct_cast_test(test_s2s_trivial_fields)
add_struct_cast_test(test_s2s_union_fields)
add_struct_cast_test(test_s2s_variable_buffer_fields)
add_struct_cast_test(test_s2s_vec_of_recs_fields)
# temporary tests with out catch for easier debugging in gdb
add_executable(hello_s2s ${TEST_DIR}/hello_s2s.cpp)
target_include_directories(hello_s2s PRIVATE ${catch2_SOURCE_DIR}/single_include)


enable_testing()
add_test(NAME s2s_arr_of_recs_fields COMMAND test_s2s_arr_of_recs_fields)
add_test(NAME s2s_computation_from_fields COMMAND test_s2s_computation_from_fields)
add_test(NAME s2s_field_validators COMMAND test_s2s_field_validators)
add_test(NAME s2s_fixed_buffer_fields COMMAND test_s2s_fixed_buffer_fields)
add_test(NAME s2s_magic_fields COMMAND test_s2s_magic_fields)
add_test(NAME s2s_optional_fields COMMAND test_s2s_optional_fields)
add_test(NAME s2s_trivial_fields COMMAND test_s2s_trivial_fields)
add_test(NAME s2s_union_fields COMMAND test_s2s_union_fields)
add_test(NAME s2s_variable_buffer_fields COMMAND test_s2s_variable_buffer_fields)
add_test(NAME s2s_vec_of_recs_fields COMMAND test_s2s_vec_of_recs_fields)

