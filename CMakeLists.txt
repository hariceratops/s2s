project(struct_cast)
cmake_minimum_required(VERSION 3.20)

set(SINGLE_HEADER_GENERATOR "AMALGAM" CACHE STRING "Single header generator, choices: AMALGAM, QUOM")

# Add Catch2
Include(FetchContent)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.x
)
FetchContent_MakeAvailable(catch2)


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

# Add the test executable
add_executable(test_s2s_arr_of_recs_fields ${TEST_DIR}/test_s2s_arr_of_recs_fields.cpp)
target_link_libraries(test_s2s_arr_of_recs_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_arr_of_recs_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_computation_from_fields ${TEST_DIR}/test_s2s_computation_from_fields.cpp)
target_link_libraries(test_s2s_computation_from_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_computation_from_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_field_validators ${TEST_DIR}/test_s2s_field_validators.cpp)
target_link_libraries(test_s2s_field_validators PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_field_validators PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_fixed_buffer_fields ${TEST_DIR}/test_s2s_fixed_buffer_fields.cpp)
target_link_libraries(test_s2s_fixed_buffer_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_fixed_buffer_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_magic_fields ${TEST_DIR}/test_s2s_magic_fields.cpp)
target_link_libraries(test_s2s_magic_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_magic_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_optional_fields ${TEST_DIR}/test_s2s_optional_fields.cpp)
target_link_libraries(test_s2s_optional_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_magic_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_trivial_fields ${TEST_DIR}/test_s2s_trivial_fields.cpp)
target_link_libraries(test_s2s_trivial_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_trivial_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_union_fields ${TEST_DIR}/test_s2s_union_fields.cpp)
target_link_libraries(test_s2s_union_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_union_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_variable_buffer_fields ${TEST_DIR}/test_s2s_variable_buffer_fields.cpp)
target_link_libraries(test_s2s_variable_buffer_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_variable_buffer_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

add_executable(test_s2s_vec_of_recs_fields ${TEST_DIR}/test_s2s_vec_of_recs_fields.cpp)
target_link_libraries(test_s2s_vec_of_recs_fields PRIVATE Catch2::Catch2)
target_include_directories(test_s2s_vec_of_recs_fields PRIVATE ${catch2_SOURCE_DIR}/single_include)

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

