# [test] Length-prefixed buffers move to test/fields/ and gain compile-time parity

Covers `str_field` and `vec_field` — the descriptors whose width comes from the
size axis rather than the type. Same shape as 029.

Sources today: `test/runtime/variable_buffer_fields.cpp`,
`test/runtime/write_variable_buffer_fields.cpp`, and the corresponding portions
of the `constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/variable_buffer_{read,write}.cpp` (GoogleTest) and
  `test/fields/variable_buffer_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `str_field` and `vec_field` each have at least one **read**-side compile-time
  case.
- Compile-time coverage includes a length drawn from a sibling field, and the
  aliased-length case where several fields depend on one length.
- Old sources deleted; `ctest` green.
