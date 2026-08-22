add_ut_test(harness_ct)
add_ut_test(traits_ct)
add_ut_test(field_list_metadata_ct)
add_ut_test(containers_ct)

# The positive form of 048's "existing suite passes unchanged" criterion: what
# the tag-to-field pipeline actually builds for a union's alternatives. Lives
# here rather than under schema/ because it asserts a type, not a round-trip.
add_ut_test(union_choice_pipeline_ct)
