# [chore] Constraints and validation — the eleven-construct DSL

The constraint axis is a whole DSL with eleven constructs in
`include/field_validation/field_value_constraints.hpp` and no prose anywhere in
either documentation file. `UserGuide.md`'s table of contents promises a
"Validating field members" section that does not exist.

Depends on: 014.

## Acceptance Criteria
- All eleven constructs are documented: `eq`, `neq`, `lt`, `gt`, `lte`, `gte`,
  `any_of`, `range`, `is_in_open_range`, `is_in_closed_range`, `no_constraint`.
- Open versus closed range semantics are stated explicitly rather than left to
  be guessed from the names.
- `no_constraint` is documented as the default every descriptor carries when the
  constraint axis is not given.
- It is stated that constraints are checked in both directions — in place while
  reading, and before the field's first byte is emitted while writing — and that
  a violation yields `validation_failure` in either case.
- The relationship between the magic descriptors and `eq` is shown: a
  `magic_string` is a `fixed_string_field` carrying an `eq` constraint, not a
  separate mechanism.
- Any non-trivial example is backed by a registered CTest target under
  `test/single_header/`.
