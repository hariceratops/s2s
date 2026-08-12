# [test] Union fields move to test/fields/ and gain compile-time parity

Covers `variance<id, deducer>` and the alternative-selection machinery — the
largest runtime suite in the repo (10 read cases, 10 write cases). Same shape
as 029.

Sources today: `test/runtime/union_fields.cpp` (515 lines),
`test/runtime/write_union_fields.cpp`, and the corresponding portions of the
`constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/union_{read,write}.cpp` (GoogleTest) and
  `test/fields/union_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `variance` has at least one **read**-side compile-time case.
- Compile-time coverage spans the selection forms the runtime suite already
  distinguishes: switch on a discriminant, ladder of predicates, and a computed
  switch.
- Compile-time coverage includes `type_deduction_failure` — the case where no
  alternative is selectable.
- At least one alternative that is itself a record is covered at compile time.
- Old sources deleted; `ctest` green.

## Notes 2026-08-12

Done. `ctest` 89/89 (was 84; +8 entries, −3).

GoogleTest inventory unchanged at 20 cases: `S2STest` (10) → `UnionRead`,
`WriteUnionFields` (10) → `UnionWrite`. The read names were snake_case
(`variant_field_parsing_with_complex_type_predicate`) and became behaviour
sentences naming which selection form each covers.

Ported to ut and removed from `constexpr_write.cpp`: `roundtrip_union` in both
byte orders and `discriminant_is_derived`. From `constexpr_read.cpp`: the 5
`unionish` asserts.

`union_read_ct.cpp` is 9 tests, 31 asserts. The read side had one compile-time
case — a discriminant switch landing on one alternative — so all three
selection forms are now covered, each shown both selecting and failing:

- discriminant switch, both `match_case` branches plus a value matching
  neither. Selecting only ever one alternative would pass a switch that
  ignores its discriminant entirely.
- predicate ladder, first branch, a later branch whose alternative is a
  record, and no branch holding.
- computed switch, matching on a value that appears nowhere on the wire, and
  failing when the computation lands between cases.

`type_deduction_failure` naming field `"c"` is asserted in all three forms,
since each reaches it by a different route.

### `constexpr_read.cpp` is gone

Every section had migrated by the end of this slice, leaving a file whose
`main` was `return 0` and which asserted nothing — a binary that passes
because it does nothing, which is the failure mode this whole feature exists
to remove. Deleted, and dropped from `test/constexpr/CMakeLists.txt`. 041
still owns retiring the directory: `constexpr_write.cpp` keeps 8
`static_assert`s (computed lengths and fan-out), which are 037's to move.

### I emptied `union_write.cpp` and `ctest` stayed green

A one-liner in the rename script read `open(p,'w').write(open(p).read()...)`.
`open(p,'w')` truncates before the argument is evaluated, so it read back an
empty file and wrote nothing. `union_write.cpp` went to 0 bytes, all 10 cases
gone, and `ctest` reported 89/89.

An empty source still links `gtest_main` and exits 0. gtest even prints
`This test program does NOT link in any test case` — and nothing was reading
it. Fixed in `add_struct_cast_test` with a `FAIL_REGULAR_EXPRESSION` on that
message; verified by emptying the file again and watching the entry fail. This
is the GoogleTest mirror of the `_coverage` check 028 added for ut, and the
same class of bug: a binary that passes because it does nothing.

It does not catch a *partial* loss — cases deleted from a file that still has
others. That needs an expected-count manifest, which is 041's coverage-audit
territory if it is worth doing at all.
