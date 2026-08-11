# [test] Rewrite static_tests_1.cpp as a registered ut traits suite

`test/constexpr/static_tests_1.cpp` is unregistered in CMake and written
against an API several generations old (`field<"x", int, 4>` for today's
`field<id, T, field_size<...>, constraint>`, `typelist::typelist` for
`typelist::list`). It asserts over traits in
`include/field_validation/field_value_constraints_traits.hpp`, which no header
includes, so those traits never reach the amalgamated single header at all.

This absorbs the `dev/inbox/todos.md` item "Static test cases for all type
traits and concepts" and forces the decision on the orphaned header. It also
takes in the two constexpr files that test machinery rather than a schema
construct: `containers.cpp` and `field_list_metadata.cpp`.

Depends on: 028.

## Acceptance Criteria
- The traits and concepts suite lives under `test/fields/` (or a sibling for
  non-descriptor machinery, if design prefers) as a ut source, registered via
  `add_ut_test`, and runs in both modes.
- Every assertion is rewritten against the current API. No assertion is carried
  over unmodified without confirming it still names something that exists.
- `containers.cpp` and `field_list_metadata.cpp` are folded in as ut suites,
  and their existing `static_assert`s port 1:1.
- `static_tests_2.cpp`'s `meta::type_id` assertions are folded in or explicitly
  retained, not silently dropped.
- `field_value_constraints_traits.hpp` is resolved: either included from a
  header the amalgamation reaches — in which case its traits are covered by
  this suite — or deleted. The choice and its reasoning are recorded in a
  `## Notes` section.
- The `todos.md` traits item is ticked off, with a pointer to this issue.
- No unregistered test source remains under the old `test/constexpr/`.
- `ctest` green.
