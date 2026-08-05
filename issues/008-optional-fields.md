# [feat] Optional fields round-trip with presence verification

Support `maybe<field, parse_if<callable, with_fields<...>>>`. An optional's
presence is a predicate over sibling fields, not a stored flag, so there is
nothing to make read-only and nothing to derive — presence can only be
verified. The struct's actual `has_value()` must agree with what the presence
predicate computes from the sibling data being written.

Depends on: 001.

## Acceptance Criteria
- An optional field whose presence predicate evaluates true and which holds a
  value round-trips, emitting the value's bytes.
- An optional field whose presence predicate evaluates false and which holds
  no value round-trips, emitting nothing for that field.
- When the predicate evaluates true but the optional is empty, the write fails
  with a validation-style `cast_error` naming the field.
- When the predicate evaluates false but the optional holds a value, the write
  fails the same way — writing nothing would silently discard caller data.
- The predicate is evaluated against the struct being written, using the
  existing presence-checking machinery rather than a parallel implementation.
- `always_present` optionals round-trip without special-casing.
- Sibling fields feeding a presence predicate remain assignable — 005's
  read-only constraint must not capture them.
- Round-trip and both mismatch cases verified in `test/runtime/`; round-trip
  also in `test/constexpr/`.
