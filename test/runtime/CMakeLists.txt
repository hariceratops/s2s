# Add GoogleTest
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


add_struct_cast_test(arr_of_recs_fields)
add_struct_cast_test(computation_from_fields)
add_struct_cast_test(field_validators)
add_struct_cast_test(fixed_buffer_fields)
add_struct_cast_test(magic_fields)
add_struct_cast_test(optional_fields)
add_struct_cast_test(trivial_fields)
add_struct_cast_test(union_fields)
add_struct_cast_test(variable_buffer_fields)
add_struct_cast_test(vec_of_recs_fields)

