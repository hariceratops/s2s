add_struct_cast_test(trivial_read)
add_struct_cast_test(trivial_write)
add_ut_test(trivial_read_ct)
add_ut_test(trivial_write_ct)

# Relocated from test/runtime/trivial_fields.cpp, which held a nested-record
# case. 032 fills this out.
add_struct_cast_test(record_read)
