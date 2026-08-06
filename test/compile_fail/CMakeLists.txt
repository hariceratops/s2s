# Compile-fail harness (issue 005). No such harness existed before the write
# path; derived fields are enforced by a constraint, so the only way to test
# them is to assert that a translation unit fails to compile.
#
# Each negative case is built as its own excluded-from-ALL target; ctest drives
# the build and expects it to fail. The positive case must build cleanly.

function(add_compile_fail_case name source case_id)
  add_executable(${name} ${source})
  set_target_properties(${name} PROPERTIES
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  target_compile_definitions(${name} PRIVATE CASE=${case_id})
  add_dependencies(${name} generate_single_header)
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${name} --config $<CONFIG>)
  set_tests_properties(${name} PROPERTIES WILL_FAIL TRUE)
endfunction()

function(add_compile_pass_case name source case_id)
  add_executable(${name} ${source})
  set_target_properties(${name} PROPERTIES
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  target_compile_definitions(${name} PRIVATE CASE=${case_id})
  add_dependencies(${name} generate_single_header)
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${name} --config $<CONFIG>)
endfunction()

add_compile_fail_case(compile_fail_derived_len derived_field_assignment.cpp 1)
add_compile_pass_case(compile_pass_derived_const_read derived_field_assignment.cpp 3)
# TODO(issue 009): discriminant case, once variance write support lands.
# add_compile_fail_case(compile_fail_derived_discriminant derived_field_assignment.cpp 2)
