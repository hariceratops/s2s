include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

function(add_struct_cast_test name)
  add_executable(${name} ${name}.cpp)
  add_dependencies(${name} generate_single_header)
  target_link_libraries(${name} PRIVATE gtest gtest_main)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

set(RUNTIME_TEST_DIR "runtime")
set(CONSTEXPR_TEST_DIR "constexpr")
set(COMPILE_FAIL_TEST_DIR "compile_fail")
set(SINGLE_HEADER_TEST_DIR "single_header")

add_subdirectory(${RUNTIME_TEST_DIR})
add_subdirectory(${CONSTEXPR_TEST_DIR})
add_subdirectory(${COMPILE_FAIL_TEST_DIR})
add_subdirectory(${SINGLE_HEADER_TEST_DIR})
