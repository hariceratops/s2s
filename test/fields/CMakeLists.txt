add_struct_cast_test(trivial_read)
add_struct_cast_test(trivial_write)
add_ut_test(trivial_read_ct)
add_ut_test(trivial_write_ct)

add_struct_cast_test(fixed_buffer_read)
add_struct_cast_test(fixed_buffer_write)
add_ut_test(fixed_buffer_read_ct)
add_ut_test(fixed_buffer_write_ct)

add_struct_cast_test(variable_buffer_read)
add_struct_cast_test(variable_buffer_write)
add_ut_test(variable_buffer_read_ct)
add_ut_test(variable_buffer_write_ct)

# Relocated from test/runtime/variable_buffer_fields.cpp, which held a
# computed-length case. 037 fills this out.
add_struct_cast_test(size_axis_read)

add_struct_cast_test(record_read)
add_struct_cast_test(record_write)
add_ut_test(record_read_ct)
add_ut_test(record_write_ct)

add_struct_cast_test(magic_read)
add_struct_cast_test(magic_write)
add_ut_test(magic_read_ct)
add_ut_test(magic_write_ct)

# Relocated from test/runtime/write_magic_fields.cpp, which held a plain
# constraint case. 036 fills this out.
add_struct_cast_test(constraint_write)

add_struct_cast_test(optional_read)
add_struct_cast_test(optional_write)
add_ut_test(optional_read_ct)
add_ut_test(optional_write_ct)

add_struct_cast_test(union_read)
add_struct_cast_test(union_write)
add_ut_test(union_read_ct)
add_ut_test(union_write_ct)
