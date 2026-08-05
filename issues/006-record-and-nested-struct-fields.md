# [feat] Record and nested struct fields round-trip

Support field kinds whose elements are themselves `struct_field_list`s,
requiring the writer to recurse into a nested field list rather than writing
raw bytes.

Depends on: 001, 004 (vector-of-records needs derived lengths).

## Acceptance Criteria
- `struct_field` (a nested `struct_field_list`) round-trips.
- `array_of_records` (`std::array<T, N>` where `T` is `field_list_like`)
  round-trips.
- `vector_of_records` (length-prefixed vector of `field_list_like`)
  round-trips, with its length derived per 004.
- Recursion preserves strict left-to-right ordering at every nesting level.
- Constraint checkers on nested fields run during the recursive write, and a
  nested violation fails the whole write with `failed_at` identifying the
  nested field.
- Nested length fields inside record elements derive from their own element's
  data, not the outer struct's.
- Round-trip verified for both byte orders, in both `test/runtime/` and
  `test/constexpr/`.
