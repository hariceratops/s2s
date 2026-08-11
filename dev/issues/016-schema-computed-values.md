# [chore] Schema: computed values — with_fields, compute, parse_if, predicate

`with_fields` appears 46 times in the test tree, `parse_if` 18, `predicate` 14,
`compute` directly 5. `parse_if` and `len_from_fields` cannot be written without
naming `with_fields`, so this family is already load-bearing public surface
rather than an `api/` implementation leak.

Depends on: 014, 015.

## Acceptance Criteria
- `with_fields` is documented as the combinator that names the sibling fields a
  computation reads.
- `compute` is documented in its generic form `compute<callable, R, Fs>` as a
  user-facing mechanism for deriving any value from sibling fields, with equal
  billing to the aliases built on top of it — not as an implementation detail
  behind them.
- `predicate`, `parse_if`, `len_from_fields`, and `size_from_fields` are
  documented as aliases of that generic form, so the relationship is visible
  rather than left for a reader to infer.
- The section states that documenting the generic form is a stability
  commitment: the signature of `compute<callable, R, Fs>` is held stable going
  forward, not just the `parse_if`/`len_from_fields` aliases (PRD Decisions §1).
- At least one worked example uses the generic form directly, backed by a
  registered CTest target under `test/single_header/`.
