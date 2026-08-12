# [test] Constraints move to test/fields/ and gain compile-time parity

Covers the `constraint` axis — the fourth axis of every descriptor, applied
in-place during reading and checked before writing. Same shape as 029, but
cross-cutting rather than tied to one descriptor.

Sources today: `test/runtime/field_validators.cpp` (7 cases: eq, neq, lt, gt,
lte, gte, any_of) plus the constraint-related portions of the `constexpr/`
files.

Depends on: 028.

## Acceptance Criteria
- Tests live at `test/fields/constraint_{read,write}.cpp` (GoogleTest) and
  `test/fields/constraint_{read,write}_ct.cpp` (ut, dual-mode). This is the one
  slice where the GoogleTest files may legitimately end up empty and be
  omitted: `field_validators.cpp`'s seven cases are the only existing
  GoogleTest cases that are constant-evaluable, so they all move to ut. If a
  GoogleTest file here has no cases, it is not created — an empty placeholder
  target is worse than an absent one.
- Routing is by constant-evaluability, not by whether a file is opened: a
  body the compiler can evaluate on its own goes to ut, everything else
  (both `ifstream` and `stringstream` bodies) stays GoogleTest. Nothing is
  in both.
- Per-feature GoogleTest suite name; no catch-all suite names survive the move.
- Existing `static_assert`s port 1:1, adding no coverage.
- Pass/fail results identical before and after the move.
- Every constraint form documented in `docs/constraints.md` has at least one
  compile-time case on the read side, covering both the satisfied and the
  violated outcome.
- A violated constraint yields a compile-time-observable
  `error_reason::validation_failure` naming the offending field.
- `no_constraint` — the default — is covered, so the absence of a constraint is
  a tested state rather than an assumed one.
- Old sources deleted; `ctest` green.

## Notes 2026-08-12 (from 033)

`test/fields/constraint_write.cpp` already exists, holding the one relocated
case (`RejectsConstraintViolationOnAnOrdinaryField`, from
`write_magic_fields.cpp`). Create nothing; fill it out.

