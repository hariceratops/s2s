# [feat] Value-based size axis, behind the existing spelling

The size axis is expressed as types today (`field_size<fixed<2>>`), which is
what forces the size to stay a `typename` parameter while `constraint_on_value`
is already an `auto` NTTP — and that mismatch is what blocks both a defaulted
size and an order-independent option pack. Turning sizes into values removes
the blockage.

This issue makes the internal representation value-based while leaving the
user-facing spelling exactly as it is. Normalization at the alias boundary
absorbs today's wrapped forms, so nothing under `test/`, `docs/`, or
`examples/` changes and the suite proves the rewrite preserved behaviour rather
than being rewritten alongside it. The new surface and the tree-wide migration
follow in 045.

In scope: the `_B` user-defined literal in `s2s_literals`, value shapes for
`len_from_field` and `size_from_fields`/`len_from_fields`, and re-derivation of
every trait keyed on the old type-level shape —
`is_fixed_size`/`is_variable_size`/`is_computed_size`/`is_selectable_size` in
`field_size.hpp`; `len_obligation`/`discriminant_obligation`/
`conditional_len_obligation`/`union_len_obligation` in `derived_value.hpp`,
which pattern-match whole `field<...>` specialisations; and
`field_size_deduce.hpp` / `comptime_field_size_deduce.hpp`.

The spec is explicit that a trait resisting the value-based shape is solved
here rather than filed as a follow-up. `size_choices` is touched only insofar
as unavoidable — its declared shape is not a requirement, and
`dev/issues/027` stays pending on the outcome rather than being fixed.

Spec: `dev/specs/schema-api-verbosity.md`.

## Acceptance Criteria
- `2_B` exists in `s2s_literals` and denotes two bytes; no bit- or
  KB-denominated literal is introduced.
- `len_from_field` and `size_from_fields`/`len_from_fields` are values, leaving
  the trailing parameters uniformly `auto` NTTPs.
- Every trait listed above matches the value-based shape; none still pattern-
  matches `field_size<fixed<N>>` as a type.
- No file that spells a *schema* outside `include/` is modified: all of
  `test/schema/`, `test/doc_examples/`, `examples/`, `docs/`,
  `test/shipped_header/`, and the rest of `test/internals/` stay byte-identical.
  That is what this criterion protects — the suite proving behaviour must not be
  rewritten alongside the thing it proves.
- The one sanctioned exception is `test/internals/traits_ct.cpp:35-41`, which
  asserts the trait *shape* being replaced rather than any behaviour: it mixes
  value arguments (`is_fixed_size_v<field_size<fixed<4>>>`) with type arguments
  (`is_fixed_size_v<int>`) and reaches into `field_size<fixed<6>>::size_type_t::count`.
  No choice of trait parameter kind satisfies both halves, and nothing preserves
  `size_type_t` once the count lives in a value. Rewrite that block to
  `decltype`-shaped assertions plus a `deduce_field_size` check. A white-box
  trait test necessarily moves with the trait it inspects; this is not schema
  spelling and so does not engage the spec's atomic-migration requirement.
- `single_header/s2s.hpp` is regenerated here too, not only in 045 — otherwise
  `test/shipped_header/` builds the pre-044 tree. Regeneration is required
  whenever `include/` moves.
- The full suite passes otherwise unchanged, including `*_compile_time` and
  `*_coverage` — `struct_field_list` remains usable in constant evaluation on
  gcc 14+ with no regression.
- `dev/issues/027` is left pending, neither fixed nor closed.
