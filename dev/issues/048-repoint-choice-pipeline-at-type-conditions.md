# [chore] Re-point the choice pipeline at the type-condition list

`to_field_choices` builds a union's per-alternative fields by zipping
`std::variant<types...>` against a separate `size_choices_t<sizes...>`,
positionally. Two parallel lists transport exactly two things, which is the
structural reason an alternative can carry neither a constraint nor a bound:
there is no third list, and adding one would mean a fourth the next time an
option is added.

This re-points it at the type-condition pack itself — `match_case`/`branch`
both already expose `::type_tag` — so each alternative's options travel on its
own tag rather than in a list positionally aligned with it. Nothing
user-visible changes: tags gain `::constraint` and `::bound` at fixed defaults
(`no_constraint<type>{}` and `use_default_bound`), which is exactly what
`to_field_choice` hardcodes today.

The point of doing this first, alone, is that it is behaviour-preserving and
the existing suite is the proof. Every slice after it changes a spelling or
adds a capability; this one changes neither, so a regression here is
unambiguous.

It also removes `size_choices_t`'s last consumer inside the library
(`type_deducer::sizes` has exactly one — `field.hpp:108`; `type.hpp:22,29,37`,
`switch.hpp:14` and `ladder.hpp:15` only forward it). That leaves
`size_choices_t` a purely user-facing type, which is what
`dev/issues/027-is-selectable-size-trait-reads-wrong-trait.md` is open about.
027 is unblocked by this, not resolved by it.

Spec: `dev/specs/union-alternatives-have-no-option-pack.md`.

## Acceptance Criteria
- `to_field_choices` reads `type`, `size`, `constraint` and `bound` off each
  case's `type_tag`, rather than zipping a variant against a `size_choices_t`.
  No parallel per-option choices list is added.
- Every type tag exposes `::constraint` and `::bound`. In this slice both are
  fixed defaults — `no_constraint<tag::type>{}` and `use_default_bound` — so
  the fields built are identical to the ones built today.
- The constraint's type argument is the tag's `::type` (`std::vector<T>`, not
  `T`), matching what `constraint_option_like` is checked against when a pack
  is resolved in a later slice.
- **The existing test suite passes unchanged, with no edits to any test.**
  Needing to edit a test means this was not behaviour-preserving and the slice
  is wrong, not the test.
- No tag signature changes and no schema in `test/`, `examples/` or `docs/` is
  touched. Migration is 049's job and belongs in its own commit.
- `extract_field_choices`, `has_unique_field_choices` and
  `has_unique_match_values` are unaffected — they pattern-match
  `type_switch<match_case<...>...>` directly, independent of this pipeline.
  Confirm rather than assume.
- `size_choices_t` retains no consumer inside `include/` other than its own
  traits. If one survives, name it here rather than leaving it implicit.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
