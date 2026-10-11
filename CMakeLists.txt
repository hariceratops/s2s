project(struct_cast)
cmake_minimum_required(VERSION 3.20)

set(SINGLE_HEADER_GENERATOR "AMALGAM" CACHE STRING "Single header generator, choices: AMALGAM, QUOM")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The runtime path builds on gcc 13, but constexpr use of struct_field_list
# does not: gcc 13 rejects the static constexpr table initializers in
# field_list_metadata with "accessing value of 'key' through a glvalue in a
# constant expression". gcc 14 compiles them cleanly.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
  message(FATAL_ERROR
    "gcc ${CMAKE_CXX_COMPILER_VERSION} cannot compile the constexpr path; gcc 14 or newer is required. "
    "Configure with -DCMAKE_CXX_COMPILER=g++-14.")
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Off by default so the developer build is unchanged. It is an option rather
# than flags passed through CMAKE_CXX_FLAGS because the set() above shadows that
# cache entry, which would drop them silently and leave a green run that
# sanitized nothing. See `cmake --workflow --preset ubsan` in the README.
option(S2S_SANITIZE_UNDEFINED "Build and link everything with -fsanitize=undefined" OFF)
if(S2S_SANITIZE_UNDEFINED)
  if(MSVC)
    message(FATAL_ERROR "S2S_SANITIZE_UNDEFINED needs gcc or clang; MSVC has no -fsanitize=undefined.")
  endif()
  add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all)
  add_link_options(-fsanitize=undefined)
endif()

# Set the paths
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SINGLE_HEADER_DIR "${CMAKE_SOURCE_DIR}/single_header")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
set(EXAMPLES_DIR "${CMAKE_SOURCE_DIR}/examples")
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

include(CTest)
enable_testing()
add_subdirectory(${TEST_DIR})
add_subdirectory(${EXAMPLES_DIR})

