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

## Notes 2026-08-07
- The criterion "`failed_at` identifying the nested field" is **not met**, as
  design §5 (Error propagation) already predicted. `rw_result` carries no
  name, and the outermost fold is the only place one is attached, so a nested
  violation names the outer record field. This matches read-side behaviour
  exactly (`field_reader.hpp` discards the inner `failed_at` the same way).
  Delivering the inner name means giving `cast_error` a path rather than a
  name, which changes the read path too. **Deferred 2026-08-07** to the "Error
  ergonomics" backlog item in `doc/todos.md`, which carries the design context.
