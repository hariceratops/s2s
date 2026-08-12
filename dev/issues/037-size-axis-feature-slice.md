# [test] Size axis and computed values move to test/fields/ and gain compile-time parity

Covers the size axis forms in `docs/schema/size-axis.md` — `fixed<N>`,
`len_from_field`, `size_from_fields`/`len_from_fields`, `size_choices`,
`size_dont_care` — and the derived/computed field behaviour built on them.
Same shape as 029.

Sources today: `test/runtime/computation_from_fields.cpp`,
`test/runtime/write_computed_length_fields.cpp` (7 cases including fan-out and
non-invertible lengths), and the corresponding portions of the `constexpr/`
files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/size_axis_{read,write}.cpp` (GoogleTest) and
  `test/fields/size_axis_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- Every row of `docs/schema/size-axis.md`'s form table has at least one
  compile-time case on the read side.
- Compile-time coverage includes shared-length fan-out (several dependents on
  one length field) and the contradiction case
  (`error_reason::found_contradicting_length`).
- Compile-time coverage includes a derived field being read-only — the
  behaviour `test/compile_fail/derived_field_assignment.cpp` asserts
  negatively — from the positive side.
- Old sources deleted; `ctest` green.

## Notes 2026-08-12 (from 031)

`test/fields/size_axis_read.cpp` already exists, holding the one relocated
`len_from_fields` case. Create nothing; fill it out.

031 deferred shared-length fan-out here rather than covering it twice — this
issue owns the contradiction case, which is the same behaviour from the failing
side. The fan-out `static_assert`s are still in `constexpr_write.cpp`
(`roundtrip_fanout`, `write_contradicting_fanout`), along with the computed
ones (`roundtrip_computed`, `write_disagreeing_computed`).
