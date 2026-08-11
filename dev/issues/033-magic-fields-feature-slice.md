# [test] Magic fields move to test/fields/ and gain compile-time parity

Covers `magic_byte_array`, `magic_string`, `magic_number` — the descriptors
that check their value as part of reading. Same shape as 029.

Sources today: `test/runtime/magic_fields.cpp`,
`test/runtime/write_magic_fields.cpp`, and the corresponding portions of the
`constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/magic_{read,write}.cpp` (GoogleTest) and
  `test/fields/magic_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `magic_byte_array`, `magic_string`, and `magic_number` each have at least one
  **read**-side compile-time case.
- Compile-time coverage includes the rejection path for each of the three, not
  only the matching path — a magic field that never fails is not tested.
- A rejected magic value produces a compile-time-observable `cast_error`
  naming the offending field.
- Old sources deleted; `ctest` green.
