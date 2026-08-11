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
