include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

# The header ships as a file literally named `ut`, with no extension, hence
# DOWNLOAD_NO_EXTRACT. Pinned by tag and hash both: the tag says which release,
# the hash says the tag did not move.
FetchContent_Declare(
  qlibs_ut
  URL https://raw.githubusercontent.com/qlibs/ut/v2.1.6/ut
  URL_HASH SHA256=80afe19304e2401df66f842a3698a87a56d694572014a0e32a51dc854e58ac01
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(qlibs_ut)

# Named qlibs_ut, not ut, so it cannot be confused with boost-ext/ut. SYSTEM
# keeps the header out of the -Wall -Wextra -Wpedantic the root applies.
add_library(qlibs_ut INTERFACE)
target_include_directories(qlibs_ut SYSTEM INTERFACE ${qlibs_ut_SOURCE_DIR})

function(add_struct_cast_test name)
  add_executable(${name} ${name}.cpp)
  add_dependencies(${name} generate_single_header)
  target_link_libraries(${name} PRIVATE gtest gtest_main)
  # Most of these tests write the same test_input.bin fixture, so they need a
  # working directory each or ctest -j races them.
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${name}.dir)
  add_test(NAME ${name} COMMAND ${name}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${name}.dir)
endfunction()

# One source, two builds. ut's compile-time mode is a static_assert inside the
# TU, so a compile-time failure is a build failure; its run-time mode aborts
# from a static destructor. Three entries: the first two keep those apart, the
# third checks every test really did run at compile time.
function(add_ut_test name)
  add_executable(${name}_compile_time ${name}.cpp)
  target_compile_definitions(${name}_compile_time PRIVATE UT_COMPILE_TIME_ONLY)
  target_link_libraries(${name}_compile_time PRIVATE qlibs_ut)

  add_executable(${name}_run_time ${name}.cpp)
  target_compile_definitions(${name}_run_time PRIVATE UT_RUN_TIME_ONLY)
  target_link_libraries(${name}_run_time PRIVATE qlibs_ut)

  # Neither macro: ut then runs both modes and its summary carries the
  # compile-time count, which is what the coverage check compares.
  add_executable(${name}_coverage ${name}.cpp)
  target_link_libraries(${name}_coverage PRIVATE qlibs_ut)

  # The compile-time binary does nothing when run — its assertions were spent
  # during compilation. Running it would report a stale binary from the last
  # good build as a pass, so the entry drives the build instead. It builds the
  # coverage target too, so that check cannot read a stale summary either.
  add_test(NAME ${name}_compile_time
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
            --target ${name}_compile_time ${name}_coverage --config $<CONFIG>)
  set_tests_properties(${name}_compile_time PROPERTIES
    RESOURCE_LOCK build_tree
    FIXTURES_SETUP ${name}_built)

  # No fixture on this one: it is the build that still links when a
  # compile-time expectation fails, which is exactly when it is wanted.
  add_test(NAME ${name}_run_time COMMAND ${name}_run_time)

  add_test(NAME ${name}_coverage
    COMMAND ${CMAKE_COMMAND} -DBINARY=$<TARGET_FILE:${name}_coverage>
            -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ct_coverage_check.cmake)
  set_tests_properties(${name}_coverage PROPERTIES
    FIXTURES_REQUIRED ${name}_built)
endfunction()

set(FIELDS_TEST_DIR "fields")
set(RUNTIME_TEST_DIR "runtime")
set(CONSTEXPR_TEST_DIR "constexpr")
set(COMPILE_FAIL_TEST_DIR "compile_fail")
set(SINGLE_HEADER_TEST_DIR "single_header")
set(INTERNALS_TEST_DIR "internals")

add_subdirectory(${FIELDS_TEST_DIR})
add_subdirectory(${RUNTIME_TEST_DIR})
add_subdirectory(${CONSTEXPR_TEST_DIR})
add_subdirectory(${COMPILE_FAIL_TEST_DIR})
add_subdirectory(${SINGLE_HEADER_TEST_DIR})
add_subdirectory(${INTERNALS_TEST_DIR})
