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

## Notes 2026-08-12

Done. `ctest` 67/67 (was 60; +9 entries, −2).

GoogleTest inventory unchanged at 10 cases: `S2STest` (3) →
`VariableBufferRead` (2) + `SizeAxisRead` (1), `WriteVariableBufferFields` (7)
→ `VariableBufferWrite` (7). Ported to ut and removed from
`constexpr_write.cpp`: 6 `static_assert`s (`roundtrip_prefixed<false>`,
`<true>`, `derived_length_bytes`, and the three on
`write_overlong_container`).

`variable_buffer_read_ct.cpp` is new — 6 tests, 22 asserts: both descriptors
sized from a sibling length, both in foreign byte order (the length slot and
the elements are separate byte-order decisions, and a string must not reverse),
a zero length giving an empty container, and a length longer than the bytes
that remain.

### These descriptors are readable at compile time after all

`constexpr_read.cpp` carries a commented-out block saying a `vector_of_records`
read "will not work since vector leaks into runtime". That is true of what it
was doing — a namespace-scope `constexpr auto fields = *parse()`, where the
allocation has to survive constant evaluation. Inside a function returning
`bool` the allocation is transient and the read is legal, which is what a ut
test body already is. Probed both `str_field` and `vec_field` before writing
the suite. `docs/compile-time.md` calls these descriptors disqualifying without
drawing that distinction; worth a look in the 039 docs sweep.

### One criterion moved to 037

This issue and 037 both claimed shared-length fan-out. 037 owns it: it also
owns the contradiction case (`found_contradicting_length`), which is the same
behaviour seen from the failing side, and splitting the pair across two slices
would be worse than either home. Same reasoning sent the third read case
(`len_from_fields` over two fields) to `test/fields/size_axis_read.cpp` early,
following 029's `record_read` precedent. What stays here is `len_from_field` —
the single-sibling length, which no `str_field` or `vec_field` can be declared
without.
