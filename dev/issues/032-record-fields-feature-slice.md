# [test] Record fields move to test/fields/ and gain compile-time parity

Covers the three descriptors that nest a schema inside a schema:
`struct_field`, `array_of_records`, `vector_of_records`. Same shape as 029.

Sources today: `test/runtime/arr_of_recs_fields.cpp`,
`test/runtime/vec_of_recs_fields.cpp`, `test/runtime/write_record_fields.cpp`,
and the corresponding portions of the `constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/record_{read,write}.cpp` (GoogleTest) and
  `test/fields/record_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `struct_field`, `array_of_records`, and `vector_of_records` each have at
  least one **read**-side compile-time case.
- Compile-time coverage includes recursion into a nested schema and both byte
  orders, since endianness is handled per leaf rather than per record.
- Old sources deleted; `ctest` green.
