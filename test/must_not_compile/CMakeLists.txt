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

# A rejected case that must fail for a stated reason. The diagnostic matched is
# our own static_assert text rather than the compiler's concept wording, so
# matching it stays portable in the way matching a substitution failure would
# not. PASS_REGULAR_EXPRESSION replaces WILL_FAIL: the build still has to fail,
# since nothing in these cases compiles, but a failure for an unrelated reason
# no longer passes — which is the hole WILL_FAIL alone leaves.
function(add_rejected_case_matching name source case_id regex)
  add_executable(${name} ${source})
  set_target_properties(${name} PROPERTIES
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  target_compile_definitions(${name} PRIVATE CASE=${case_id})
  add_dependencies(${name} generate_single_header)
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${name} --config $<CONFIG>)
  set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${regex}")
endfunction()

add_rejected_case(rejects_assigning_a_derived_discriminant derived_field_assignment.cpp 2)
add_rejected_case(rejects_reading_a_hidden_length_target hidden_length_target.cpp 1)
add_rejected_case(rejects_duplicate_match_values derived_field_assignment.cpp 4)

add_rejected_case(rejects_unrecognised_pack_entry option_pack_misuse.cpp 1)
add_rejected_case(rejects_duplicate_size_option option_pack_misuse.cpp 2)

add_rejected_case(rejects_bound_on_fixed_size_field option_pack_misuse.cpp 3)
add_rejected_case(rejects_duplicate_bound_option option_pack_misuse.cpp 4)

# The tag option pack (049). Each was checked against a no-CASE control build
# and fails on its own diagnostic: 5 on the duplicate-size assertion, 6 on
# variable_size_like with an empty pack, 7 on field_fits_to_underlying_type —
# which is what proves as_trivial's relocated requires-clause still constrains.
add_rejected_case(rejects_duplicate_size_on_tag option_pack_misuse.cpp 5)
add_rejected_case(rejects_unsized_as_string option_pack_misuse.cpp 6)
add_rejected_case(rejects_oversized_as_trivial option_pack_misuse.cpp 7)

# The per-alternative constraint (050). Checked against a no-CASE control build:
# 8 fails on the duplicate-constraint assertion, not on the tag's arity.
add_rejected_case(rejects_duplicate_constraint_on_tag option_pack_misuse.cpp 8)

# The per-alternative bound (051). Each checked against a no-CASE control build:
# 9, 11 and 12 fail on the per-element placeholder constraint, which is where
# "a bound needs a variable size" is enforced; 13 on the duplicate-count
# assertion.
add_rejected_case(rejects_bound_on_as_struct option_pack_misuse.cpp 9)
add_rejected_case(rejects_bound_on_as_trivial option_pack_misuse.cpp 11)
add_rejected_case(rejects_bound_on_as_fixed_arr option_pack_misuse.cpp 12)
add_rejected_case(rejects_duplicate_bound_on_tag option_pack_misuse.cpp 13)

# The union-level constraint (052). variance takes a constraint-only pack, so
# 10 and 14 fail on the per-element placeholder constraint and 15 on the
# duplicate-count assertion. Each checked against a no-CASE control build.
# A field frozen by an eq constraint. Both checked against a no-CASE control
# build: each fails as assign-to-const on the read-only overload, which is what
# proves the field stays readable rather than disappearing the way a length
# target does.
add_rejected_case(rejects_assigning_a_frozen_magic_field derived_field_assignment.cpp 5)
add_rejected_case(rejects_assigning_a_frozen_eq_field derived_field_assignment.cpp 6)

add_rejected_case(rejects_size_on_variance option_pack_misuse.cpp 10)
add_rejected_case(rejects_bound_on_variance option_pack_misuse.cpp 14)
add_rejected_case(rejects_duplicate_constraint_on_variance option_pack_misuse.cpp 15)

# The byte-order axis (053-060). Checked against a no-CASE control build: 7
# fails on announcing_record_check's name-resolution assertion, not on the
# guide's arity or on order_deduction_like.
add_rejected_case_matching(rejects_unknown_announcement_field_name
  byte_order_misuse.cpp 7 "names a field the announcing record does not have")

# The order-agnostic restriction (056). 3 is a u16 declared directly in the
# announcing record, 4 the same u16 one record deeper — which is what proves the
# walk descends rather than reading one flat pack. Both fail on the
# order-agnostic assertion, and both now say so rather than merely failing.
add_rejected_case_matching(rejects_order_dependent_field_in_announcing_record
  byte_order_misuse.cpp 3 "every field in it must read the same under either byte order")
add_rejected_case_matching(rejects_order_dependent_field_nested_deeper
  byte_order_misuse.cpp 4 "every field in it must read the same under either byte order")

# The exactly-once rejections (057). 6 is an announcement reached conditionally,
# 10 one reached repeatedly. Matching the census's off-spine sentence is what
# distinguishes them from a failure on the maybe's or the array's own
# constraints, which is exactly what they must not fail on.
add_rejected_case_matching(rejects_conditionally_reached_announcement
  byte_order_misuse.cpp 6 "must be read exactly once, unconditionally")
add_rejected_case_matching(rejects_repeatedly_reached_announcement
  byte_order_misuse.cpp 10 "must be read exactly once, unconditionally")

# The entry points (058). Each names the mismatch and the call the caller
# wanted, so both are matched against that sentence rather than merely expected
# to fail.
add_rejected_case_matching(rejects_struct_cast_on_a_fixed_order_schema
  byte_order_misuse.cpp 1 "call struct_cast_le or struct_cast_be to read it")
add_rejected_case_matching(rejects_struct_cast_le_on_a_self_announcing_schema
  byte_order_misuse.cpp 2 "call struct_cast to read it, or stream_cast to write it")

# The write entry points (059), mirroring 1 and 2. stream_cast_le on a
# self-announcing schema is the one that matters: it would emit bytes in an
# order the marker sitting in the same struct contradicts.
add_rejected_case_matching(rejects_stream_cast_on_a_fixed_order_schema
  byte_order_misuse.cpp 8 "or stream_cast_le or stream_cast_be to write it")
add_rejected_case_matching(rejects_stream_cast_le_on_a_self_announcing_schema
  byte_order_misuse.cpp 9 "or stream_cast to write it")

# At most one announcement (060). Matched against the sentence rather than
# merely expected to fail, because the wording is the point: the message has to
# say limitation, not conflict.
add_rejected_case_matching(rejects_more_than_one_announcing_record
  byte_order_misuse.cpp 5 "current limitation of s2s, not an inherent conflict")

# The discovered-length size axis (062-064). Checked against a no-CASE control
# build: 1 fails on delimited_buffer_is_byte_wide, not on any other option-pack
# rule.
add_rejected_case(rejects_wide_element_delimited_vector delimited_misuse.cpp 1)
