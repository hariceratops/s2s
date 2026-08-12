# Not a test directory, but registered with CTest all the same: this example
# spent its life unbuilt in test/hello/ and went stale unnoticed. Building it
# is not sufficient either — it has to run, and it has to be able to fail.

add_executable(hello_s2s hello_s2s.cpp)
add_dependencies(hello_s2s generate_single_header)

# Only the shipped header, the way a consumer sees it.
target_include_directories(hello_s2s PRIVATE ${SINGLE_HEADER_DIR})
target_compile_options(hello_s2s PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-non-template-friend)

add_test(
  NAME make_example_fixture
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/make_sample.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(make_example_fixture PROPERTIES FIXTURES_SETUP example_fixture)

add_test(NAME hello_s2s COMMAND hello_s2s
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(hello_s2s PROPERTIES FIXTURES_REQUIRED example_fixture)
