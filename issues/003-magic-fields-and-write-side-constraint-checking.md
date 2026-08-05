# [feat] Magic fields and write-side constraint checking

Run each field's `constraint_checker` on the write path, exactly as the read
path does, so a struct holding a value that violates its constraint fails
fast rather than emitting a stream that will not read back. Magic fields are
the field kinds where this matters most.

Depends on: 001.

## Acceptance Criteria
- `magic_string`, `magic_number`, and `magic_byte_array` round-trip.
- Every field's `constraint_checker` is evaluated before that field is
  written, for all field kinds, not only magic ones.
- A struct holding a value that fails its `constraint_checker` (e.g. a wrong
  magic value) fails the write with a validation-style `cast_error` whose
  `failed_at` names the offending field.
- The failing write is rejected before the offending field's bytes reach the
  stream.
- Round-trip and failure cases verified for both byte orders, in both
  `test/runtime/` and `test/constexpr/`.
