# [chore] Schema: maybe and variance — presence and type deduction

The two combinators that sit on top of the four axes. Presence deduction
(`maybe` with `parse_if`) and type deduction (`variance` with `type_switch` /
`match_case` / `if_else_ladder`) have no prose anywhere in either file, despite
optionals and unions both appearing in the README feature list.

Depends on: 016.

## Acceptance Criteria
- `maybe` is documented with presence deduction: how `parse_if` decides whether
  the field is present on read, and how presence is verified against
  `has_value()` on write.
- `variance` is documented with type deduction, covering both selection
  mechanisms: `type_switch` with `match_case` for discriminant-driven selection,
  and `if_else_ladder` for predicate-driven selection.
- The `match_field` discriminant of a `type_switch` is identified as invertible
  and therefore derived on write; an `if_else_ladder` is identified as not
  invertible and therefore verified. Consequences are cross-referenced to
  Writing rather than restated.
- `type_deduction_failure` is explained: no ladder branch and no `match_case`
  matched at all.
- Worked examples for both combinators are backed by registered CTest targets
  under `test/single_header/`.
