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

## Notes 2026-08-12

Done. `ctest` 71/71 (was 67; +7 entries, −3).

GoogleTest inventory unchanged at 10 cases: `RecordRead` (1, already relocated
by 029) + `MetaStructTest` (1) + `S2STest` (2) merged into `RecordRead` (4),
and `WriteRecordFields` (6) → `RecordWrite` (6). Three runtime sources
collapse into two — `arr_of_recs_fields.cpp` and `vec_of_recs_fields.cpp` were
one and two cases each, split by descriptor rather than by feature.

Ported to ut and removed from `constexpr_write.cpp`: `roundtrip_records`
(both byte orders) and `nested_bytes_are_in_declaration_order`. From
`constexpr_read.cpp`: the four `array_of_records` `static_assert`s.

### Two dead things the read side was carrying

`parse_our_recs_struct` — the `struct_field` parser — was defined and never
asserted on. The nested-record read had no compile-time coverage at all
despite looking like it did. It is now `record_read_ct.cpp`'s first test.

`vector_of_records` was commented out with "will not work since vector leaks
into runtime". Same misreading 031 found: the leak is a namespace-scope
`constexpr auto` holding the vector, not the read. Uncommented as a ut test
and it passes at compile time.

Also removed a `point` alias in `constexpr_write.cpp` left unused once the
record section moved out.

`record_read_ct.cpp` is 6 tests, 31 asserts. Beyond the 1:1 port and the two
revived cases: both byte orders for `struct_field` and `array_of_records`
(endianness is per leaf, so the inner fields must reverse individually rather
than the record's bytes as a block), and a buffer ending inside an element,
which fails the whole read rather than returning the elements before it.
