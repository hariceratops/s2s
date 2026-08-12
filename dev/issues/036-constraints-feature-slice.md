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


## Notes 2026-08-12

Done. `ctest` 94/94 (was 89; +6 entries, −1).

`test/runtime/field_validators.cpp` is deleted, not moved: all seven cases
were constant-evaluable, so they became ut tests. No `constraint_read.cpp`
GoogleTest file was created, per this issue's own criterion — an empty
placeholder target is worse than an absent one. `constraint_write.cpp` keeps
its single relocated case from 033.

The seven ported cases test the constraint *objects*, calling them as
predicates. That is not the same as a read consulting one, so each of the
eight documented forms also gets a read that satisfies it and a read that
violates it, going through `struct_cast_le` with the value on the wire. Both
groups are in `constraint_read_ct.cpp`, separated by a comment — 17 tests, 42
asserts.

`constraint_write_ct.cpp` is 7 tests, 29 asserts. It is entirely new: the
write side had no constraint coverage at compile time beyond what the magic
descriptors happened to exercise.

Two behaviours that had no coverage in either direction:

- Exhaustion outranks a constraint that would also have failed. The check runs
  after the field is read, so a short buffer has to keep `buffer_exhaustion`
  rather than report the constraint.
- A violated constraint stops the read at that field, so the field behind it
  is not populated from the wire.

Boundaries are asserted on the rejecting side for every ordering form: `lt`
rejects its own value, `gte` accepts it. A form tested only well away from its
boundary would pass with an off-by-one comparison.

### Documented exception: range constraints

`docs/constraints.md` §"Range constraints do not currently compile" documents
`range`, `is_in_open_range` and `is_in_closed_range` as present in the headers
and **not usable** — `range`'s constructor `static_assert`s on its own
arguments, so it is a hard compile error at any use. They therefore have no
compile-time case here, and cannot have one until 026 lands. This is the one
row of that page with no coverage, and it is deliberate.
