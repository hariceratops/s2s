# [test] Optional fields move to test/fields/ and gain compile-time parity

Covers `maybe<field, present_only_if>`, whose presence is a predicate over
sibling fields rather than a stored flag. Same shape as 029.

Sources today: `test/runtime/optional_fields.cpp` (8 cases),
`test/runtime/write_optional_fields.cpp` (13 cases), and the corresponding
portions of the `constexpr/` files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/optional_{read,write}.cpp` (GoogleTest) and
  `test/fields/optional_{read,write}_ct.cpp` (ut, dual-mode).
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- `maybe` has at least one **read**-side compile-time case.
- Compile-time coverage includes both predicate outcomes — present and absent —
  and a wrapped field that is not a trivial (the existing runtime suite covers
  optionals over arrays, strings, vectors and records; the compile-time tier
  covers at least one non-trivial wrapped kind).
- `always_present` is covered at compile time.
- Old sources deleted; `ctest` green.
