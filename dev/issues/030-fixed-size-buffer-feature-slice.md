# [test] Fixed-size buffers move to test/fields/ and gain compile-time parity

Covers the four const-sized descriptors: `fixed_array_field`, `c_arr_field`,
`fixed_string_field`, `c_str_field`. Same shape as 029.

Sources today: `test/runtime/fixed_buffer_fields.cpp`,
`test/runtime/write_fixed_buffer_fields.cpp`, and the corresponding portions of
the `constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/fixed_buffer_{read,write}.cpp` (GoogleTest) and
  `test/fields/fixed_buffer_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- Each of `fixed_array_field`, `c_arr_field`, `fixed_string_field`, and
  `c_str_field` has at least one **read**-side compile-time case.
- Compile-time coverage includes per-element byteswapping (not per-buffer) and
  the nested-aggregate case, since those are the behaviours the runtime suite
  already treats as load-bearing.
- Compile-time coverage includes the terminator accounting for the string
  descriptors (`N + 1` bytes consumed).
- Old sources deleted; `ctest` green.
