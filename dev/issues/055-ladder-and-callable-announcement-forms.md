# [feat] the ladder and callable forms of a byte-order announcement

054 wires only `match_field`. The type-deduction axis supports three input
forms, and a byte-order announcement wants all three for the same reason type
deduction does: an if-else ladder expresses conditions a single match cannot,
and a `compute_t` callable over several named fields lets the order depend on
more than one field of the announcing record — which is the whole reason the
brief chose to mark the record rather than the field.

`compute_t` already carries a callable plus its required field names, and
`extract_type_deduction_dependencies` already pulls those names into the
dependency table. Both are reused rather than duplicated for this axis.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- The if-else ladder form resolves a byte order from an announcing record.
- The `compute_t` callable form resolves a byte order from several named fields
  of the announcing record.
- Required field names are extracted through the existing
  `extract_type_deduction_dependencies` path, not a parallel one, and each is
  checked for existence against the **announcing record's own** field table.
- A name that is not a field of that record is a compile error, with a
  diagnostic naming it.
- No entry is added to the enclosing list's dependency table.
  `is_dependencies_resolved` dereferences a lookup in *that* list's field table,
  and an announcement's names are fields of the inner one — see
  `dev/design/byte-order-is-data-not-a-template-parameter.md` §3.6. The ordering
  half of dependency resolution is vacuous on this axis: an announcement's names
  are its own siblings, all read before it completes.
- A test proves a two-field deduction — one a single `match_field` could not
  express.
- All three forms produce identical reads for a schema where they are
  equivalent.
- Covered in both test tiers.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 054.
