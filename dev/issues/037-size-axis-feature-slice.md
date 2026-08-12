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

## Notes 2026-08-12

Done. `ctest` 98/98 (was 94; +6 entries, −2).

GoogleTest inventory unchanged at 9 cases: `SizeAxisRead` (1 relocated by 031)
+ `RuntimeComputationTest` (1) → `SizeAxisRead` (2), and
`WriteComputedLengthFields` (7) → `SizeAxisWrite` (7).

Ported to ut: the last 8 `static_assert`s in `constexpr_write.cpp` — the
computed-length and fan-out cases 031 deferred here. **`constexpr_write.cpp`
is deleted**, and with `constexpr_read.cpp` gone in 035, no
`constexpr_cast`-style file remains. `test/runtime/` is deleted entirely, and
`test/CMakeLists.txt` no longer references it. 041 still has `test/constexpr/`,
which holds `containers.cpp`, `field_list_metadata.cpp` and the two
`static_tests_*` files — none of them cast tests.

### Form table walk (`docs/schema/size-axis.md`)

| Form | Compile-time read case |
|---|---|
| `fixed<N>` | `trivial_read_ct.cpp`, and every other ut suite |
| `len_from_field<"id">` | `variable_buffer_read_ct.cpp` (031) |
| `size_from_fields` / `len_from_fields` | `size_axis_read_ct.cpp`, new here |
| `size_choices<...>` | **none — not declarable, see below** |
| `size_dont_care` | `record_read_ct.cpp` (032), plus an explicit case here |

041 re-walks this; it is recorded now so that walk is a check rather than a
rederivation.

### Documented exception: `size_choices`

027 reports `is_selectable_size_v` reading `is_fixed_size<T>::res`, so
`selectable_size_like` rejects the one type it exists for and a
`field_size<size_choices<...>>` cannot be declared. Re-probed rather than
taken on trust: a `basic_field` with that size fails to compile with 3 errors
on gcc 14.2. It therefore has no compile-time case and cannot until 027 lands.
The library's own union machinery builds `size_choices` types internally and
consumes them directly, which is why the suite is green without it.

Second deliberate exception in the feature, after range constraints in 036.

### New read-side coverage

`size_axis_read_ct.cpp` is 6 tests, 23 asserts; the read side had no
computed-length or fan-out compile-time coverage at all.

- A computed length where the count appears nowhere on the wire, and a second
  case changing one source to a different count — a single case would pass if
  something other than the callable were supplying the length.
- Fan-out where the two dependents hold different element widths, so a shared
  length is shown to be a count rather than a byte size.
- A derived field read through the const accessor, which is the positive half
  of what `test/compile_fail/derived_field_assignment.cpp` asserts negatively.
- `size_dont_care` with a field behind the nested schema, so the width really
  did come from that schema.
