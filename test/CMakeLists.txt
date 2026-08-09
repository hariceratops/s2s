
set(RUNTIME_TEST_DIR "runtime")
set(CONSTEXPR_TEST_DIR "constexpr")
set(COMPILE_FAIL_TEST_DIR "compile_fail")
set(SINGLE_HEADER_TEST_DIR "single_header")

add_subdirectory(${RUNTIME_TEST_DIR})
add_subdirectory(${CONSTEXPR_TEST_DIR})
add_subdirectory(${COMPILE_FAIL_TEST_DIR})
add_subdirectory(${SINGLE_HEADER_TEST_DIR})

