# Schema DSL verbosity: value-based sizes and wrapper elision

PRD. Supersedes the design-conversation brief this file replaced. That brief
was never committed, so git does not retain it; its substance survives here,
chiefly in Non-Goals, which carry the directions it rejected and why.
Flavor: minimal — single-maintainer library, no external users of
`single_header/s2s.hpp` to coordinate with.

## Overview

A schema declaration should read as the format it describes, not as the
metaprogramming machinery that implements it. Today a two-byte field is
spelled `basic_field<"version", u16, field_size<fixed<2>>>`; only the name,
type, and `2` are load-bearing. This rewrite collapses that to
`basic_field<"version", u16, 2_B>` by making `field_size<>`/`with_fields<>`
wrappers optional (elided by a normalizing metafunction at each user-facing
alias), defaulting a `basic_field`'s size to `sizeof(T)`, turning sizes into
values so they join an order-independent trailing option pack, and hiding
`len_from_field` targets from the struct API entirely, since the write path
derives them and no assignment can keep them honest. The rewrite reaches
every trait the size axis touches — nothing is left matching the old
type-level shape — and lands as one atomic migration of the whole tree.

## Goals

- `field_size<T>` and `with_fields<ids...>` accept either the wrapped or the
  naked spelling at every user-facing alias, including inside `variance`
  (`match_field`, `match_case`, `type_switch`, `branch`, `predicate`,
  `as_*` tags) — reached depth-first, not stopped at `variance`'s boundary.
- `basic_field`'s size parameter defaults to `sizeof(T)` when omitted, and
  the trailing parameters (size, constraint) become an order-independent
  option pack classified by kind rather than fixed position.
- A fixed-width size is a value (`2_B`, via a `_B` user-defined literal in
  `s2s_literals`, bytes only), not a type — `len_from_field`, `size_from_fields`
  /`len_from_fields` follow it into being values so the whole trailing pack is
  homogeneous `auto` NTTPs.
- Every trait keyed on the old type-level size is re-derived to match the
  value-based shape, with no deferral: `is_fixed_size`, `is_variable_size`,
  `is_computed_size`, `is_selectable_size` (`field_size.hpp`);
  `len_obligation`, `discriminant_obligation`, `conditional_len_obligation`,
  `union_len_obligation` (`derived_value.hpp`); `field_size_deduce.hpp` and
  `comptime_field_size_deduce.hpp`. A trait that resists this shape is solved
  here, not filed as a follow-up.
- An option-pack entry that isn't recognized as a size or a constraint is
  rejected by a named concept stating what was expected (`static_assert`
  only where a concept can't express the shape) — this is a delivered
  requirement, not later polish.
- `len_from_field` targets are removed from `struct_field_list::operator[]`
  entirely (both the mutable and the current const-read overload) — not
  merely made const-only, which is what the field list already does today.
  `match_field` discriminant targets are explicitly left visible and
  untouched (see Non-Goals).
- `struct_field_list` remains usable in constant evaluation on gcc 14+ with
  no regression at any point in the rewrite — the compile-time test tier
  (`add_ut_test`'s three-way build, `test/ct_coverage_check.cmake`) depends
  on it structurally, not just as a nice property.
- The old fully-wrapped spelling is removed, not deprecated, and the
  migration off it is a single atomic slice: every file under `include/`,
  `test/`, `docs/`, and `examples/` that spells a size or option today (72
  files as of this writing) moves together, old and new spellings never
  coexisting in the tree.
- `single_header/s2s.hpp` is regenerated via `scripts/amalgam.py` inside that
  same slice, so `test/shipped_header/` never sits on a stale artifact.
- `dev/issues/027-is-selectable-size-trait-reads-wrong-trait.md` is marked
  pending on this rewrite rather than left reading as a fix ready to apply
  against a trait shape that may not survive.

Testable/done criteria: `ctest` is green tree-wide post-migration, including
`*_compile_time` and `*_coverage` entries; a `test/must_not_compile/` case
demonstrates the named-concept diagnostic for an unrecognized pack entry; a
`test/must_not_compile/` case demonstrates that a `len_from_field` target no
longer has an `operator[]` overload at all; `doc_examples_match` and
`shipped_header` CTest targets pass against the regenerated docs and header.

## Non-Goals

- **No `cast_error` changes.** This spec introduces no new failure mode and
  reuses `found_contradicting_length` as-is — a length contradiction can only
  come from `size_from_fields`, whose source fields are non-invertible and
  therefore already stay visible and assignable. The coupling the original
  brief asserted between this work and `cast_error` enrichment was
  overstated; that enrichment belongs entirely to
  `dev/inbox/unbounded-resize-from-wire-length.md`.
- **No allocation-bound / resize work.** `read_native`, `field_reader.hpp`'s
  `resize` call, and any bound on a wire-supplied length are untouched here.
- **`size_choices` / issue 027 stays open, not fixed.** `size_choices` may
  itself be reshaped by this rewrite; whether that changes what "fixing"
  issue 027 means is left for after this lands, not decided now.
- **`match_field` discriminant targets stay visible.** Hiding them the way
  `len_from_field` targets are hidden is deferred — how a caller should read
  a `variance` field's held alternative at all (`std::get` by type, by
  position, or by an explicit annotation) is a live, separate design
  question this spec must not pre-empt. Every wrapper-elision change inside
  `variance`'s internals is spelling-only and behavior-preserving.
- **No new combinator sugar** for `maybe` or `variance` beyond the mechanical
  wrapper elision applied uniformly.
- **No per-case shorthand aliases** (e.g. a `len_from` beside
  `len_from_field` beside `len_from_fields`).
- **No assignment-time syncing** of derived values, and **no aggregate or
  positional construction** of `struct_field_list`.
- **No bit- or KB-denominated size literals.** Sizes stay byte-denominated;
  bit granularity is anticipated future work (bitfields) this spec neither
  designs for nor forecloses.
- **No extensibility hook** in the option-pack classification for a
  caller-defined size or constraint kind — the classification only needs to
  recognize the current closed set.
- **No dual-spelling backward compatibility.** The old fully-wrapped
  spelling is not required to keep compiling once the migration lands.

## Technical Approach

- C++23, header-only, template-metaprogramming EDSL; no new runtime
  dependency. Touches `include/api/field_descriptors.hpp`,
  `include/field/`, `include/field_size/`, `include/field_list/`,
  `include/field_write/derived_value.hpp`, and `include/type_deduction/`
  (the `variance` machinery), plus every consumer under `test/` and `docs/`.
- **Value-based sizes.** A `_B` UDL joins the existing `s2s_literals`
  namespace (already home to the `"id"_f` field accessor literal) and
  produces an `auto` NTTP. `len_from_field<id>` and
  `size_from_fields<f, ids>`/`len_from_fields` become values of their own
  (currently-empty) types so `basic_field`'s trailing parameters — size and
  `constraint_on_value` — are uniformly `auto` NTTPs, closing the gap that
  currently forces the size to stay a type.
- **Wrapper elision.** A normalizing metafunction at each user-facing alias
  (`basic_field`, `fixed_array_field`, `vec_field`, `str_field`, `maybe`,
  `variance`'s `as_*` tags, etc.) wraps a naked spec into whatever internal
  representation the traits below still match on, so the traits' pattern
  matches do not need to know whether the caller wrote the wrapper.
- **Trait re-derivation.** Every size-axis trait currently pattern-matching
  `field_size<fixed<N>>`/`field_size<len_from_field<id>>`/etc. as *types* is
  rewritten to match the value-based shape (via `decltype` or direct value
  comparison, whichever a given trait needs) — `field_size.hpp`'s
  `is_fixed_size`/`is_variable_size`/`is_computed_size`/`is_selectable_size`,
  `derived_value.hpp`'s four obligation traits, and the two
  `field_size_deduce` headers. `size_choices` is included in this pass only
  insofar as touching it is unavoidable; its declared shape is not a
  requirement of this spec (see Non-Goals).
- **Option-pack classification.** A named concept (e.g. something in the
  shape of `size_option_like` / `constraint_option_like`) sorts each trailing
  NTTP by kind; an entry matching neither fails a top-level concept
  requiring it be one or the other, so the diagnostic names the pack
  position and the two things it could have been. `static_assert` is a
  fallback only where a concept genuinely can't express the shape being
  rejected.
- **Hiding `len_from_field` targets.** `field_list_metadata.hpp` currently
  merges length-derived and discriminant-derived field ids into one
  `derived_field_ids` list, and `field_list.hpp`'s `operator[]` gives every
  entry in it a const-only (not absent) overload. This spec splits that
  bucket in two — length-derived vs. discriminant-derived — since the two
  now diverge in treatment: a length-derived id gets no `operator[]`
  overload at all (a plain "no such member" compile error), while a
  discriminant-derived id keeps today's const-read behavior unchanged.
- **Migration mechanics.** The 72 files currently spelling `field_size<`
  (12 in `include/`, the rest in `test/`, `docs/`, and `examples/`) move to
  the new spelling in one slice, followed by `scripts/amalgam.py`
  regenerating `single_header/s2s.hpp` and a `dev/issues/027` status edit —
  all part of the same vertical slice per the project's per-slice commit
  convention, not a follow-up cleanup.
- **Constexpr preservation.** Every new trait and the option-pack concept
  must remain valid in constant evaluation (no runtime-only construct
  introduced into a path `struct_field_list` depends on) — verified by the
  existing three-way `add_ut_test` build rather than a new mechanism.

## Open Questions

- Whether the discriminant/length split inside `derived_field_ids` is best
  expressed as two separate lists or one list with a per-entry kind tag is
  an implementation choice left to the architect — noted here only because
  the current code has exactly one bucket and this spec requires two
  behaviors out of it.
- Whether `is_selectable_size`/`size_choices` end up touched at all by the
  trait re-derivation, and in what shape, is unknown until the rewrite is
  attempted — tracked by the pending status on issue 027, not resolved here.

## Deliverables checklist

- [ ] Value-based size literal (`_B`) and value-shaped
      `len_from_field`/`size_from_fields`.
- [ ] Order-independent, concept-classified trailing option pack on
      `basic_field` (and other affected aliases).
- [ ] Wrapper elision at every user-facing alias, including inside
      `variance`.
- [ ] Full re-derivation of every size-axis trait listed above.
- [ ] `len_from_field` targets absent from `operator[]`; `match_field`
      targets unchanged.
- [ ] Atomic migration of `include/`, `test/`, `docs/`, `examples/` off the
      old spelling.
- [ ] `single_header/s2s.hpp` regenerated and `test/shipped_header/` green.
- [ ] `dev/issues/027` marked pending on this rewrite.
