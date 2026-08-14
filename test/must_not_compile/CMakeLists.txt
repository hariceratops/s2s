# Rejected-misuse harness (issue 005). No such harness existed before the write
# path; derived fields are enforced by a constraint, so the only way to test
# them is to assert that a translation unit fails to compile.
#
# Each negative case is built as its own excluded-from-ALL target; ctest drives
# the build and expects it to fail. The positive case must build cleanly.

function(add_rejected_case name source case_id)
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

add_rejected_case(rejects_assigning_a_derived_discriminant derived_field_assignment.cpp 2)
add_rejected_case(rejects_reading_a_hidden_length_target hidden_length_target.cpp 1)
add_rejected_case(rejects_duplicate_match_values derived_field_assignment.cpp 4)

add_rejected_case(rejects_unrecognised_pack_entry option_pack_misuse.cpp 1)
add_rejected_case(rejects_duplicate_size_option option_pack_misuse.cpp 2)

# Scaffolded for 047, deliberately NOT registered yet — same reason as before:
# add_rejected_case sets WILL_FAIL, so a case naming an API that does not exist
# fails to build and passes for the wrong reason, and would keep passing if the
# feature were never implemented. Uncomment when max_bytes lands.
#
# add_rejected_case(rejects_bound_on_fixed_size_field option_pack_misuse.cpp 3)
# add_rejected_case(rejects_duplicate_bound_option option_pack_misuse.cpp 4)
