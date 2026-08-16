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


add_struct_cast_test(optional_read)
add_struct_cast_test(optional_write)
add_ut_test(optional_read_ct)
add_ut_test(optional_write_ct)

add_struct_cast_test(union_read)
add_struct_cast_test(union_write)
add_ut_test(union_read_ct)
add_ut_test(union_write_ct)

# No constraint_read.cpp: field_validators.cpp's cases were all
# constant-evaluable, so they moved to ut rather than staying GoogleTest.
add_struct_cast_test(constraint_write)
add_ut_test(constraint_read_ct)
add_ut_test(constraint_write_ct)

add_struct_cast_test(size_axis_write)
add_ut_test(size_axis_read_ct)
add_ut_test(size_axis_write_ct)

# The allocation ceiling (047). Its own pair rather than additions to
# variable_buffer_read*, because it verifies an option rather than a container
# kind — the tree is organised by what a file checks.
add_struct_cast_test(allocation_bound_read)
add_ut_test(allocation_bound_read_ct)

# Two more translation units purely to vary S2S_DEFAULT_MAX_BYTES: the macro is
# per-TU and header-only s2s reads it at include time, so each file #defines its
# own ceiling before including and needs no CMake variant. ut only — the macro
# resolves at compile time and the behaviour it selects is the same code path
# the pair above already exercises against a real stream.
add_ut_test(allocation_bound_default_override_ct)
add_ut_test(allocation_bound_disabled_ct)

# Options declared on a union alternative (050, 051). Its own pair for the same
# reason allocation_bound_* has one: the tree is organised by what a file
# verifies, and this verifies an option on an alternative rather than the
# variance construct. Read and write are separate files rather than one
# round-trip, because 050's two constraint call sites must be shown to fail
# independently.
add_struct_cast_test(union_alternative_options_read)
add_struct_cast_test(union_alternative_options_write)
add_ut_test(union_alternative_options_read_ct)
add_ut_test(union_alternative_options_write_ct)
