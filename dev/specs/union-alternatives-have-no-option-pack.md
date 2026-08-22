# Union alternatives carry no options

PRD. Supersedes the design-conversation brief this file replaced (same
filename); the brief's substance survives here, chiefly in the reasoning
behind Non-Goals and the Technical Approach.
Flavor: minimal — single-maintainer library, no external users of
`single_header/s2s.hpp` to coordinate with.

## Overview

Every ordinary field descriptor (`vec_field`, `struct_field`, `basic_field`,
...) takes an order-independent trailing pack of options — a constraint, and
since `dev/inbox/unbounded-resize-from-wire-length.md` landed (now on `main`
at `a378e60`), a `max_bytes` bound. A `variance` alternative cannot take
either: `to_field_choice` in `include/field/field.hpp` hardcodes
`no_constraint<T>` for every alternative and has no bound parameter at all,
and `union_field` itself hardcodes `no_constraint<variant>` for the union as
a whole. A `match_case` whose payload should be validated, or whose vector
payload should raise or lower the default allocation ceiling, cannot say so
— not because anyone decided alternatives shouldn't have options, but
because the tag-to-field pipeline that builds them only ever transported a
type and a size.

This work gives every type tag (`as_vec`, `as_trivial`, `as_struct`, ...) the
same order-independent option pack `field_descriptors.hpp` already gives
ordinary descriptors, reaches the resulting constraint and bound through the
read and write paths for both individual alternatives and the union as a
whole, and re-points the tag-to-field pipeline at the type-condition list
itself so no parallel per-option list has to be invented — now or the next
time an option is added.

## Goals

- Every type tag takes the same order-independent trailing option pack
  `field_descriptors.hpp`'s descriptors already take, reusing that pack's
  concepts (`field_option_like`, `size_option_like`, `constraint_option_like`,
  `bound_pack_option_like`) rather than parallel ones:
  - `as_trivial<T, opts...>`, `as_vec<T, opts...>`, `as_string<opts...>`:
    size, constraint, and (for `as_vec`/`as_string`) bound all join the pack,
    order-independent among themselves — mirroring `field_descriptors.hpp`'s
    shape exactly, not keeping size positional. `as_vec<u8, max_bytes<4096>,
    len_from_field<"n">>` and `as_vec<u8, len_from_field<"n">,
    max_bytes<4096>>` are both valid, and both replace the size-only spelling
    `as_vec<u8, len_from_field<"n">>` `as_vec` had before this work — that
    spelling still compiles unchanged since the pack degrades to
    size-only with no constraint or bound supplied.
  - `as_fixed_arr<T, N, opts...>` / `as_fixed_string<N, opts...>`: `N` stays
    positional (an element count, not a size value) and gains a
    constraint-only trailing pack, mirroring `fixed_array_field` /
    `fixed_string_field`.
  - `as_struct<T, opts...>` / `as_arr_of_records<T, N, opts...>`: gain a
    constraint-only pack (no size, no bound — there is no wire-driven
    allocation to bound), mirroring `struct_field` / `array_of_records`.
  - `as_vec_of_records<T, opts...>`: full size + constraint + bound pack,
    mirroring `vector_of_records`.
  - A bound option on a type tag with no wire-driven allocation
    (`as_struct`, `as_arr_of_records`, and the fixed-size tags) is rejected
    at compile time by the same per-element concept mechanism
    `bound_pack_option_like` already uses in `field_descriptors.hpp` — not a
    runtime check, not a bolted-on `static_assert`.
- A per-alternative constraint is checked on both the read and write path.
  Since the struct-level fold (`struct_cast_impl.hpp`,
  `stream_cast_impl.hpp`) never reaches a union's inner alternative field
  (it only ever sees the outer `union_field`, whose own constraint is
  separately handled by the next goal), the check is invoked explicitly at
  `read_variant_impl` and `write_variant_impl`
  (`include/field_read/field_reader.hpp`,
  `include/field_write/field_writer.hpp`), the same way `maybe_field`'s
  constraint — also rewritten to `no_constraint` over the wrapper — is
  checked explicitly rather than through the fold.
- A per-alternative bound is threaded through to whatever allocation-bound
  check `dev/inbox/unbounded-resize-from-wire-length.md` put in place for
  ordinary variable-sized fields, so an oversized alternative is rejected
  with a `cast_error` before allocation, exactly as an oversized ordinary
  `vec_field`/`str_field`/`vector_of_records` field already is. Where no
  bound is declared, an alternative keeps defaulting to `default_max_bytes`,
  as it silently does today.
- `variance` as a whole — not only its alternatives — can carry a
  constraint, checked against the resolved variant value (e.g. "the parsed
  value must be one of these specific alternatives, not just well-typed").
  `union_field`'s own base field currently hardcodes
  `no_constraint<variant>{}` at `include/field/field.hpp:103`; this becomes
  a pack option on `variance` itself, applied at the same fold point every
  other struct-level field's constraint is applied at (`struct_cast_impl.hpp`
  / `stream_cast_impl.hpp`), since a `union_field`'s own value — unlike its
  alternatives' — already passes through that fold.
- `to_field_choices` (`include/field/field.hpp`) is re-pointed at the
  type-condition list (`match_case`/`branch` pack) directly, reading each
  case's `type_tag::type`, `::size`, `::constraint`, and `::bound`, instead
  of zipping `std::variant<types...>` against a separate `size_choices_t`.
  No third or fourth parallel per-option list is added alongside it. This is
  a requirement on the pipeline's shape — alternatives carry their own
  resolved options, there is exactly one place that reads them — not a
  mandate on the exact template signature; the design phase picks the
  precise spelling.
- A test covers reading and writing a constrained alternative, a bounded
  alternative, a rejected-at-compile-time bound on a non-container tag, and
  a constrained `variance` as a whole, under both `test/runtime/` and
  `test/constexpr/` per `dev/specs/compile-time-test-tier.md`.

## Non-Goals

- Resolving issue 027 (declarable `size_choices`). This work removes
  `size_choices_t`'s last internal consumer (`to_field_choices`), which
  leaves 027 with a clean, purely user-facing surface to redesign — but does
  not do that redesign. Issue 027 stays open and unblocked by, not settled
  by, this work.
- Runtime-only rejection of a meaningless bound. `as_struct`/
  `as_arr_of_records`/fixed-size tags reject a bound at compile time via a
  concept, matching the precedent in `field_descriptors.hpp`
  (`rejects_bound_on_fixed_size_field`); there is no fallback runtime check
  to design.
- Extending `cast_error`. A rejected alternative constraint or bound reuses
  the existing `{failure_reason, failed_at}` shape and an existing
  `error_reason`; no new fields are added to carry which alternative failed.
- A compatibility shim for the old positional-size tag spelling. Every type
  tag's signature changes (size moves from a fixed positional slot into the
  pack); every existing union schema in `test/`, `examples/`, and `docs/`
  that spells a tag positionally is migrated in the same change, atomically
  — the same choice `dev/specs/schema-api-verbosity.md` made for the
  equivalent break, and for the same reason: the library has no external
  users of the old spelling to keep compiling.

## Technical Approach

- **Pipeline re-pointing (Finding 1).** `type_deducer::sizes` today has
  exactly one consumer, `to_field_choices` at `field.hpp:108`; every other
  site (`type.hpp:22,29,37`, `switch.hpp:14`, `ladder.hpp:15`) only
  forwards it. `to_field_choices` moves to pattern-match on the
  `type_condition_like` case pack directly (`match_case`/`branch`, both of
  which already expose `::type_tag`) and pull `type`, `size`, `constraint`,
  `bound` off each case's tag, e.g.:
  ```cpp
  template <fixed_string id, type_condition_like... cases>
  struct to_field_choices<id, type_condition_list<cases...>> {
    using choices = field_choice_list<
      field<id, typename cases::type_tag::type,
                cases::type_tag::size,
                cases::type_tag::constraint,
                cases::type_tag::bound>...>;
  };
  ```
  The exact intermediate type standing in for `type_condition_list<cases...>`
  (a new alias, or `type_switch`/`ladder` passed through directly) is a
  design-phase decision, not fixed here.
- **Type tags gain the option pack (`type_tags.hpp`).** Each tag's `::size`,
  `::constraint`, `::bound` become `resolved_options<T, opts...>` members,
  reusing `size_of_pack`/`constraint_of_pack`/`bound_of_pack` from
  `field_descriptors.hpp` rather than re-deriving them. `as_trivial`'s
  existing `requires fixed_size_like<size_type_of<S>> &&
  (deduce_field_size<S>{}() <= sizeof(T))` — currently written directly
  against the named parameter `S` — has to move to read the *resolved*
  size out of the pack instead, the same relocation `basic_field` already
  made (`requires field_fits_to_underlying_type<size_of_pack<T, opts...>,
  T>`); this is a known follow-on of making size part of the pack, not a new
  problem to discover mid-implementation.
- **Per-alternative constraint (Finding 2).** Checked explicitly at
  `read_variant_impl` (`field_reader.hpp:239`) and `write_variant_impl`
  (`field_writer.hpp:212`), following the precedent already in
  `field_writer.hpp:195` (`maybe_field`'s explicit check, with the comment
  explaining why the fold can't reach it). The type the per-alternative
  constraint is checked against is the tag's `::type` (e.g.
  `std::vector<T>`, not `T`) — the same type `constraint_option_like` is
  matched against when the pack is resolved. Worth stating plainly so the
  reference docs describe the right type when this lands.
- **Per-alternative bound (Finding 2, extended).** Threaded through the same
  read-path allocation-bound check `unbounded-resize-from-wire-length`
  introduced for ordinary variable-sized fields; an alternative's resolved
  `::bound` reaches that check the same way an ordinary field's `field_bound`
  does.
- **Union-level constraint (Finding 3).** `union_field`'s own base `field`
  gains a pack option slot on `variance` (`field_descriptors.hpp`) in place
  of the hardcoded `no_constraint<typename type_deducer::variant>{}`. This
  constraint is checked at the ordinary fold point
  (`struct_cast_impl.hpp:38`, `stream_cast_impl.hpp:32`), since — unlike an
  alternative's — the union's own value already passes through it.
- **Bound-on-meaningless-tag rejection.** `bound_pack_option_like` (or an
  equivalent per-tag concept) admits a bound only where the tag's `::size`
  is variable — mirroring how `field_descriptors.hpp` already restricts
  bounds to the three container descriptors. A registered `must_not_compile`
  case analogous to `rejects_bound_on_fixed_size_field` covers a bound on
  `as_struct`/`as_arr_of_records`/a fixed-size tag.
- No changes to `extract_field_choices`/`has_unique_field_choices`
  (`type_deduction_metafunctions.hpp`) or `has_unique_match_values` — both
  pattern-match on `type_switch<match_case<...>...>` directly, independent
  of `to_field_choices`/`size_choices_t`, so re-pointing the latter doesn't
  disturb them.

## Open Questions

None. The brief's four open questions are resolved above: the 027 dependency
runs from this work toward 027, not the reverse (Non-Goals); options are
spelled on the type tag, fully joining the pack rather than staying
positional (Goals); a per-alternative constraint is checked explicitly on
both read and write paths (Technical Approach); and a bound is rejected at
compile time on alternative kinds with no wire-driven allocation (Goals,
Non-Goals). The union-level pack, not raised in the brief at all, is in
scope (Goals).
