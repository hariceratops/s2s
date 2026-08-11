# [chore] Schema: the size axis in full

The size axis is the one axis with more than a couple of forms, and it is
undocumented beyond `fixed` and `len_from_field` appearing incidentally in
examples.

Depends on: 014.

## Acceptance Criteria
- Every size form is covered with what it means and when it applies: `fixed`,
  `len_from_field`, `len_from_fields`, `size_from_fields`, `size_choices`, and
  `size_dont_care`.
- `field_fits_to_underlying_type` is explained — why a declared width can reject
  a type at compile time.
- Sizes that are invertible (and therefore derived on write) are distinguished
  from those that are not (and therefore verified on write). The consequences
  are cross-referenced to the Writing section rather than restated here.
- Any non-trivial example is backed by a registered CTest target under
  `test/single_header/`.
