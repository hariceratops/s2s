function(add_constexpr_test name)
  add_executable(${name} ${name}.cpp)
  add_dependencies(${name} generate_single_header)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_constexpr_test(constexpr_read)
add_constexpr_test(containers)
add_constexpr_test(field_list_metadata)
add_constexpr_test(static_tests_2)

# Write path
add_constexpr_test(constexpr_write)

# TODO: static_tests_1.cpp is not registered — it is written against an API
# several generations old (`field<"x", int, 4>` for what is now
# `field<id, T, field_size<...>, constraint>`, `typelist::typelist` for
# `typelist::list`) and asserts over traits in
# include/field_validation/field_value_constraints_traits.hpp, which no header
# includes, so they never reach the amalgamated single header at all. Fixing it
# means rewriting the assertions against the current API and deciding whether
# that orphaned header should be wired in or deleted — which is the "Static
# test cases for all type traits and concepts" and "Remove dead code" items in
# dev/inbox/todos.md, not part of repairing the harness.
