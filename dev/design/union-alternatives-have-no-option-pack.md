# Design: union alternatives carry an option pack

Spec: `dev/specs/union-alternatives-have-no-option-pack.md` (frozen).
Issues served, in order:
`dev/issues/048-repoint-choice-pipeline-at-type-conditions.md`,
`dev/issues/049-type-tags-take-an-option-pack.md`,
`dev/issues/050-alternatives-carry-a-constraint.md`,
`dev/issues/051-alternatives-carry-an-allocation-bound.md`,
`dev/issues/052-variance-carries-a-constraint.md`.

Branch `feature/union-alternative-options`, cut from `main` at `a378e60`. Sits
on top of `dev/design/schema-api-verbosity.md` (043–045) and
`dev/design/unbounded-resize-from-wire-length.md` (046–047), and obeys both of
their governing rules verbatim:

- **A class-type NTTP is a non-deduced context in a partial specialization**, so
  anything a trait pattern-matches on lives in the value's *type*. Traits deduce
  values as opaque `auto` and interrogate `decltype`; `size_type_of<v>` is the
  one place cv is stripped.
- **Option classification is per-element, never a fold.** `field_option_like<T>
  auto... opts` is a placeholder constraint that substitutes `decltype(opt)`
  first, which is why every option concept takes the option's type as its first
  parameter. This design adds no new mechanism to that; it only widens *which*
  templates carry the existing one.

This design proposes **one adjustment to the agreed slice boundaries** (§6.1):
the constraint-only pack on `as_struct` / `as_arr_of_records` / `as_fixed_arr` /
`as_fixed_string` moves from 049 to 050. Everything else is served as given.

---

## 0. The shape in one page

| Piece | Where | What |
|---|---|---|
| `type_condition_list<cases...>` | **new** `type_deduction/utils/type_condition_list.hpp` | the empty list template `to_field_choices` pattern-matches on; the only new type in 048 |
| `::cases` | `switch.hpp`, `ladder.hpp`, `type.hpp` (×3) | replaces `::sizes` — a `type_condition_list`, forwarded by all three `type` specializations |
| option-pack machinery | **moved** to `field/field_options.hpp` | `size_option_like` … `resolved_options`, `*_of_pack`, `field_fits_to_underlying_type` — lifted out of `api/field_descriptors.hpp` so `type_tags.hpp` can use them without a cycle |
| `::constraint`, `::bound` | `type_tags.hpp`, all eight tags | joins `::type` and `::size`; fixed defaults in 048, pack-resolved thereafter |
| explicit per-alternative check | `field_reader.hpp` `read_variant_impl`, `field_writer.hpp` `write_variant_impl` | the fold cannot reach an alternative field; precedent is `maybe_field` at `field_writer.hpp:195` |
| third parameter on `union_field` | `field/field.hpp` | `auto constraint_on_variant = no_constraint<typename type_deducer::variant>{}` |
| `sizes`, `size_choices_from_type_conditions`, `size_from_type_condition`, `to_field_choice` | deleted in 048 | last internal consumers of `size_choices_t` |

Slice split after the §6.1 adjustment:

- **048** — re-point the pipeline. `type_condition_list`, `::cases`, tags gain
  `::constraint`/`::bound` at fixed defaults, `to_field_choices` reads all four
  off the tag. Entirely internal: nothing outside `include/` references any
  symbol it touches (verified — `to_field_choice`, `field_choice_list`,
  `is_type_tag`, `type_tag_like`, `::sizes`, `size_choices_from_type_conditions`
  have zero occurrences under `test/`, `examples/`, `docs/`). The existing suite
  is the proof, unedited.
- **049** — size joins the pack, on the four tags that have a size:
  `as_trivial`, `as_vec`, `as_string`, `as_vec_of_records`. Header split lands
  here.
- **050** — constraint joins the pack, on all eight tags (the four sizeless ones
  gain their pack here), plus the two explicit call sites.
- **051** — bound joins the pack, on `as_vec` / `as_string` /
  `as_vec_of_records`. **No read-path change at all** (§7).
- **052** — union-level constraint. Independent of 048–051; pullable forward.

---

## 1. The intermediate type `to_field_choices` matches on

**Decision: a new empty list template `type_condition_list<cases...>`, in its own
leaf header, reached through a uniform `::cases` member on all three `type`
specializations.**

```cpp
// type_deduction/utils/type_condition_list.hpp  — includes nothing
template <typename... cases>
struct type_condition_list {};
```

```cpp
// switch.hpp
template <match_case_like... cases>
  requires (sizeof...(cases) > 0)
struct type_switch {
  using variant = variant_from_type_conditions_v<cases...>;
  using conditions = type_condition_list<cases...>;
};

// type.hpp, all three specializations
using conditions = _switch::conditions;   // or ladder::conditions

// field.hpp
template <fixed_string id, typename... cases>
struct to_field_choices<id, type_condition_list<cases...>> {
  using choices = field_choice_list<
    field<id, typename cases::type_tag::type,
              cases::type_tag::size,
              cases::type_tag::constraint,
              cases::type_tag::bound>...>;
};

// union_field
using field_choices = typename to_field_choices<id, typename type_deducer::conditions>::choices;
```

Why a distinct list type rather than the alternatives:

**Passing `type_switch` / `type_if_else` through directly was the leading
alternative and is rejected on include direction.** `to_field_choices` lives in
`field/field.hpp`, whose entire include closure today is `fixed_string.hpp`,
`field_size.hpp` and `field_value_constraints.hpp`. Specializing it on
`type_switch<cases...>` and `type_if_else<branches...>` would require `field.hpp`
to see `switch.hpp` and `ladder.hpp`, dragging the whole `type_deduction/`
subtree — including `computation_from_fields_traits.hpp` via `clause.hpp` —
underneath the most-included header in the project. A one-line leaf header keeps
that closure intact. It also costs two near-identical specializations instead of
one, since a switch and a ladder are different templates.

**`type` cannot forward a pack.** A member alias cannot be a pack, so the three
`type` specializations need a single *type* to forward. That is the whole reason
a list type exists at all, and it is why `using conditions = type_switch;`
(the injected-class-name, which does work) does not help: it reintroduces the
two-specialization/include problem above.

**The pack is deliberately unconstrained in the specialization** (`typename...
cases`, not `type_condition_like... cases`). Constraining it would need
`type_condition_like` from `helper.hpp` in `field.hpp` — the same include
direction problem. Nothing is lost: `type_condition_list` is only ever
constructed by `type_switch`/`type_if_else`, both of which already constrain
their packs to `match_case_like`/`branch_like`.

`::sizes` is deleted in the same slice, along with
`size_choices_from_type_conditions`, `size_from_type_condition` and
`to_field_choice`/`to_field_choice_v`, which have no other consumer. That is
048's acceptance criterion about `size_choices_t` retaining no internal consumer;
after 048 the only things naming `size_choices_t` are its own traits
(`is_selectable_size`) and the user-facing `size_choices` variable template,
which is exactly the surface issue 027 is open about.

Naming: `conditions` rather than reusing `cases`, because `cases` is already the
parameter-pack name at three sites and a member alias shadowing it reads badly.

---

## 2. Where the option-pack machinery lives

**Decision: move it out of `api/field_descriptors.hpp` into a new
`include/field/field_options.hpp`, which both `field_descriptors.hpp` and
`type_tags.hpp` include. Lands in 049.**

The cycle is real and was measured, not assumed:

```
api/field_descriptors.hpp
  → type_deduction/type/type_deduction_traits.hpp
    → type_deduction/switch/switch_traits.hpp → switch.hpp
      → type_deduction/switch/match_case.hpp
        → type_deduction/utils/type_tags.hpp
```

so `type_tags.hpp` including `field_descriptors.hpp` closes a five-header loop.
Since these are `#ifndef` guards rather than modules, the failure would not even
be a clean "circular include" — it would be a silently-empty second inclusion
producing "`size_of_pack` is not a member of `s2s`" at a point that looks
unrelated.

What moves, and its dependency floor:

| Moves | Needs |
|---|---|
| `field_fits_to_underlying_type` | `field_size/comptime_field_size_deduce.hpp` |
| `size_option_like`, `constraint_option_like`, `field_option_like`, `bound_pack_option_like`, `boundable_field_option_like` | `field_size/field_size.hpp` (all four size concepts, `bound_option_like`) |
| `*_option_count`, `pack_options`, `*_in_pack`, `resolved_options`, `*_of_pack` | `field_validation/field_value_constraints.hpp` for `no_constraint`, `field_size.hpp` for `size_type_of` / `use_default_bound` |

None of those reach `field/field.hpp` or `type_deduction/`, so
`field/field_options.hpp` sits below both. `type_tags.hpp` already includes
`field_size.hpp` and `comptime_field_size_deduce.hpp`, so it gains one include
and loses nothing. `field_descriptors.hpp` keeps only the descriptor aliases and
`always_true`/`always_present`, which is what it is named for.

`scripts/amalgam.py` walks `include/` and topologically sorts by the include
graph, so a new header is picked up with no script change (verified).

Placement in `field/` rather than `api/`: `api/` is the user-facing surface, and
`type_deduction/` reaching up into it would invert the layering even though it
would compile. The option pack is machinery about a *field descriptor's*
parameters, and `field/` is already the layer both `api/` and `type_deduction/`
sit above.

**Rejected: leave the machinery in `field_descriptors.hpp` and duplicate the
three resolvers for tags.** That is the parallel mechanism 049's acceptance
criteria forbid by name, and it would let a fourth option kind be added to one
copy and not the other — which is precisely the failure this whole spec exists
to remove from the choice pipeline.

**Rejected: break the cycle by moving `type_tags.hpp` instead.** The cycle runs
through `match_case.hpp`, which genuinely needs `type_tag_like`; moving the tags
would just relocate the same edge.

---

## 3. Tag shapes

Final form after 051. Each tag's parameter list carries **the narrowest option
concept its kind admits** — exactly the concept its mirror descriptor alias in
`field_descriptors.hpp` already carries. That is the whole answer to "how does a
tag admit different option sets per kind while keeping the per-element
placeholder idiom": no new mechanism, no per-tag opt-in table, just the same
concept on the same kind of parameter one layer up.

```cpp
template <trivial T, field_option_like<T> auto... opts>
  requires fixed_size_like<size_type_of<size_of_pack<T, opts...>>> &&
           field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
struct as_trivial {
  using type = T;
  static constexpr auto size = size_of_pack<T, opts...>;
  static constexpr auto constraint = constraint_of_pack<T, opts...>;
  static constexpr auto bound = use_default_bound;
};

template <field_list_like T, constraint_option_like<T> auto... opts>
struct as_struct {
  using type = T;
  static constexpr auto size = size_dont_care;
  static constexpr auto constraint = constraint_of_pack<T, opts...>;
  static constexpr auto bound = use_default_bound;
};

template <trivial T, std::size_t N, constraint_option_like<std::array<T, N>> auto... opts>
struct as_fixed_arr {
  using type = std::array<T, N>;
  static constexpr auto size = byte_count{N * sizeof(T)};
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = use_default_bound;
};

template <std::size_t N, constraint_option_like<fixed_string<N>> auto... opts>
struct as_fixed_string { /* size = byte_count{N + 1}; constraint from pack */ };

template <trivial T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
struct as_vec {
  using type = std::vector<T>;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = bound_of_pack<type, opts...>;
};

template <boundable_field_option_like<std::string> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::string, opts...>>>
struct as_string { /* type = std::string; all three from the pack */ };

template <field_list_like T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
struct as_arr_of_records { /* size = size_dont_care; constraint from pack */ };

template <field_list_like T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
struct as_vec_of_records { /* all three from the pack */ };
```

Notes an implementer needs:

- **`::type` is usable in the member initializers below it, but not in the
  requires-clause.** The clause is part of the template head, where the member
  alias does not yet exist, so `as_vec`'s clause spells `std::vector<T>` while
  its members spell `type`. Uniform per-tag; not worth a helper.
- **The bound is exposed even where it cannot be declared.** `as_struct::bound ==
  use_default_bound` is inert — `to_field_choices` passes it as `field`'s fifth
  argument, which is the parameter's default, and no reader for a struct or
  fixed-size field ever reads `field_bound`. Uniform members are what let
  `to_field_choices` be a single unconditional expansion; making it conditional
  on the tag kind would put a second classification of tags into the pipeline
  the spec is trying to flatten.
- **`resolved_options` is reused through `size_of_pack`/`constraint_of_pack`/
  `bound_of_pack`, not by inheritance.** Naming any of the three instantiates
  `resolved_options` and therefore `pack_options`, so the duplicate-count
  `static_assert`s fire for tags exactly as they do for descriptors — that is
  what satisfies 049's, 050's and 051's duplicate-entry criteria with no new
  code. Inheriting from `resolved_options` was considered and rejected: the four
  sizeless tags would inherit a meaningless `size` (`byte_count{sizeof(T)}`)
  that they then have to shadow, and a shadowed inherited member is exactly the
  kind of thing a later reader mistakes for the resolved value.

### `as_trivial`'s requires-clause

```cpp
requires fixed_size_like<size_type_of<size_of_pack<T, opts...>>> &&
         field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
```

**Both conjuncts are kept deliberately**, in that order. `basic_field` carries
only the second, and that is a latent defect rather than a model to copy: the
`fixed_size_like` guard is what makes `deduce_field_size<size>{}()` inside
`field_fits_to_underlying_type` a well-formed expression at all. With a variable
size resolved out of the pack, `deduce_field_size<size>` names the primary
template, which is declared but never defined; value-initializing an incomplete
type is a substitution failure in the immediate context, so it *probably*
degrades to an unsatisfied constraint rather than a hard error — but "probably,
via SFINAE through an incomplete type" is not a property to rest a user-facing
diagnostic on, and conjunct short-circuiting makes it moot for one extra line.
The clause reads as the sentence it is: the size must be a fixed one, and it must
fit.

A consequence worth stating because it is new capability arriving as a side
effect: `as_trivial<u32>` now compiles, defaulting to `byte_count{sizeof(u32)}`
via `size_of_pack`'s default, exactly as `basic_field<"x", u32>` does. Today
`as_trivial` requires an explicit size. This is a widening, matches the mirror
descriptor, and needs a line in the docs rather than a guard.

### `as_string<opts...>` — the flagged question

**No ambiguity and no deduction problem.** Checked against each mechanism that
touches it:

1. **Use in a schema.** `as_string<len_from_field<"n">>` binds one entry to the
   pack. There is no other parameter to compete with it, so nothing is
   ambiguous — the ambiguity risk in a pack-tailed template comes from a
   *preceding* parameter that could also match, and `as_string` has none.
2. **`is_type_tag<as_string<opts...>>`**, `template <auto... opts>`. Deducing an
   `auto...` pack from a class template-id is a deduced context; the codebase
   already relies on this in the very code being replaced —
   `to_field_choices<id, std::variant<types...>, size_choices_t<sizes...>>`
   deduces `auto... sizes` out of `size_choices_t` today, and
   `dev/design/schema-api-verbosity.md` §2 records it as verified.
3. **`as_string<>` becomes syntactically legal** where `as_string<S>` previously
   forced an argument. It is rejected by the requires-clause, not silently
   accepted: with no size entry `size_of_pack<std::string>` yields
   `byte_count{sizeof(std::string)}`, which is `fixed_size_like`, so
   `variable_size_like<...>` fails. Worth one `must_not_compile` case
   (`rejects_unsized_as_string`) precisely because this is the one spelling the
   change newly admits into the grammar.
4. **`match_case<0, s2s::as_string>`** — a bare template name is not a type
   argument, so this fails exactly as it does today.

The one property that genuinely changes is that `as_string`'s entire parameter
list is now `auto...`, so *any* value-shaped argument parses and rejection moves
from the grammar to the concept. That is the same trade every other tag and every
descriptor alias already made in 045, and the per-element placeholder constraint
is what pays for it: the diagnostic names the offending entry and both things it
could have been.

### The two tags that cannot currently be used at all

`is_type_tag` has specializations for `as_trivial`, `as_fixed_arr`,
`as_fixed_string`, `as_vec`, `as_string` and `as_struct` — and **none for
`as_arr_of_records` or `as_vec_of_records`**. `match_case` and `branch` both
require `type_tag_like T`, so those two tags are documented
(`docs/schema/optional-and-variant.md:137,139`) but unusable, and nothing in
`test/` covers them. Confirmed by grep: their only occurrences anywhere are their
own definitions and those two doc table rows.

This design **fixes it in 048**, by giving both tags an `is_type_tag`
specialization in the same edit that rewrites the other six. The reasons for
doing it here rather than filing it:

- 048 rewrites every `is_type_tag` specialization anyway (each tag's parameter
  list changes shape across 049–051, and 048 is where the `::constraint`/`::bound`
  members are added). Adding two more is two lines in an edit already touching
  the same block.
- The spec's Goals list `as_arr_of_records<T, N, opts...>` and
  `as_vec_of_records<T, opts...>` taking packs. 051's criteria require
  `as_vec_of_records` to admit a bound and `as_arr_of_records` to reject one, and
  neither is testable while `match_case` refuses the tag.
- Leaving it would mean 049–051 add option packs to two templates that cannot be
  instantiated, which is untested surface by construction.

It is a behaviour change inside a slice whose criterion is "behaviour-preserving,
suite unchanged". The criterion still holds in the sense it was written for —
no existing schema changes meaning and no test is edited — but this is a
deliberate deviation and is called out as such. If the preference is to keep 048
strictly inert, the alternative is a sixth slice between 048 and 049 doing only
this; it is not worth a slice, but the choice is the human's.

**Not verified by this design:** whether the two record tags *work* once
`type_tag_like` admits them. The pipeline builds `field<id, std::array<record,
N>, size_dont_care, …>` and `field<id, std::vector<record>, size, …>`, which
`is_array_of_record_field` and `is_vector_of_record_field` do classify, so the
readers and writers exist. But `read_variant_impl` assigns
`variant = std::move(field.value)` and the write path uses `std::get<idx>`, and
nothing has ever exercised a record alternative. 048 owes a round-trip test for
each of the two tags, and if one turns out broken it is a finding for its own
issue, not a reason to widen 048.

---

## 4. Slice 048 — re-pointing the pipeline

Edits, in full:

| File | Change |
|---|---|
| `type_deduction/utils/type_condition_list.hpp` | **new**, one empty template |
| `type_deduction/utils/type_tags.hpp` | every tag gains `static constexpr auto constraint = no_constraint<type>{};` and `static constexpr auto bound = use_default_bound;`; `is_type_tag` gains the two missing specializations |
| `type_deduction/utils/helper.hpp` | delete `size_from_type_condition`, `size_from_type_condition_v`, `size_choices_from_type_conditions`, `size_choices_from_type_conditions_v` |
| `type_deduction/switch/switch.hpp`, `if_else_ladder/ladder.hpp` | `using sizes = …` → `using conditions = type_condition_list<cases...>` |
| `type_deduction/type/type.hpp` | all three specializations: `using sizes = X::sizes` → `using conditions = X::conditions` |
| `field/field.hpp` | delete `to_field_choice`/`to_field_choice_v`; `to_field_choices` re-specialized on `type_condition_list`; `union_field::field_choices` reads `type_deducer::conditions` |

Behaviour preservation, argued rather than asserted. Today's choice for case *i*
is `field<id, tag_i::type, size_i, no_constraint<tag_i::type>{}>`, where the type
comes from `variant_from_type_conditions_v` (case order) and the size from
`size_choices_t` (case order). After: `field<id, tag_i::type, tag_i::size,
no_constraint<tag_i::type>{}, use_default_bound>`. `tag_i::size` *is* `size_i` —
`size_from_type_condition` reads exactly that member — and `use_default_bound` is
`field`'s fifth default. Same types, same order, same `field_choice_list`. The
`std::variant` and `variant_size` paths are untouched.

`extract_field_choices`, `has_unique_field_choices`, `has_unique_match_values`,
`discriminant_obligation` and the six `union_field` patterns in
`field_list_metadata.hpp` all pattern-match `type_switch<match_case<v, tag>...>`
or `union_field<id, type_deducer>` directly and never go through
`to_field_choices` — **confirmed by reading each, not assumed**, per 048's
criterion. Nothing there changes in 048.

---

## 5. Slice 049 — size joins the pack

`as_trivial`, `as_vec`, `as_string`, `as_vec_of_records` take
`size_option_like<T> auto... opts` (narrowed from the final
`boundable_field_option_like`/`field_option_like`), resolving `::size` through
`size_of_pack`. `::constraint` and `::bound` stay at 048's fixed defaults, so a
constraint or bound entry is rejected by the per-element placeholder constraint
— which is the honest state of the world in this slice, since neither would run.
The header split of §2 lands here, because this is the first slice where
`type_tags.hpp` needs `size_of_pack`.

**The atomic migration 049 promises is empty, and that is expected.** Every
existing spelling is already a valid one-entry pack: `as_vec<u8,
len_from_field<"n">>`, `as_trivial<u32, 4_B>`, `as_string<len_from_field<"n">>`,
`as_fixed_arr<u8, 4>`, `as_struct<inner>` are all token-identical before and
after. The 11 files that spell a tag (`docs/schema/optional-and-variant.md`,
`test/doc_examples/{doc_example,guide_variance_example}.cpp`,
`test/internals/field_list_metadata_ct.cpp`,
`test/must_not_compile/derived_field_assignment.cpp`,
`test/schema/{size_axis_read_ct,union_read,union_read_ct,union_write,union_write_ct}.cpp`,
`test/shipped_header/roundtrip.cpp`) need no edit. Verification is therefore
`git diff --stat` showing nothing outside `include/`, `docs/` and
`single_header/` — the *absence* of a migration is the evidence, and the issue's
criterion should be read as satisfied vacuously rather than as unmet.

New negative cases: `rejects_duplicate_size_on_tag`, `rejects_unsized_as_string`,
and `rejects_oversized_as_trivial` (the last is 049's explicit "the relocated
constraint must still constrain" criterion — `as_trivial<u16, 4_B>` inside a
`match_case`).

---

## 6. Slice 050 — the per-alternative constraint

### 6.1 The one slice-boundary adjustment

**The sizeless tags' constraint-only pack belongs in 050, not 049.**

049's criteria give `as_struct<T, opts...>`, `as_arr_of_records<T, N, opts...>`,
`as_fixed_arr<T, N, opts...>` and `as_fixed_string<N, opts...>` a trailing pack
"with no size entry", while also requiring that a constraint entry be a compile
error in 049. Those two together specify a pack that admits nothing. There is no
honest concept for that — `constraint_option_like` would admit a constraint the
slice then silently drops, which is the exact failure mode 049 cites 047's
precedent to avoid, and a deliberately-false concept is a placeholder pretending
to be a rule.

So: those four tags keep their current arity through 049 and gain their pack in
050, alongside the enforcement that makes it mean something. Nothing else moves.
The rest of the 048/049 split **survives contact with the design** — 048 is
genuinely separable, because a tag can expose `::constraint = no_constraint<type>{}`
and `::bound = use_default_bound` as plain members with no pack in sight. That
was the load-bearing question and the answer is clean.

Cost of the adjustment: 050 changes four template arities instead of zero, so
049 stops being "every tag signature changes" and becomes "the four tags with a
size change". 050's own migration is still empty for the same reason 049's is.

### 6.2 The two call sites

The struct-level fold in `struct_cast_impl.hpp:37` and `stream_cast_impl.hpp:32`
runs `fields::constraint_checker` over the fields of a `struct_field_list`, and
for a union the field it sees is the outer `union_field`. The alternative's
`field` is constructed inside `read_variant_impl` / reached via `std::get` inside
`write_variant_impl` and never appears in any `struct_field_list`. Re-verified by
reading both fold sites; Finding 2 holds.

Read (`field_reader.hpp`, `read_variant_impl::read`), immediately after the
alternative reads successfully and before it reaches the variant:

```cpp
    if(!res)
      return std::unexpected(res.error());
    // The struct-level fold only ever sees the outer union_field, so this is
    // the only place an alternative's own constraint reaches its value.
    if(!T::constraint_checker(field.value))
      return std::unexpected(error_reason::validation_failure);
    variant = std::move(field.value);
```

Write (`field_writer.hpp`, `write_variant_impl::write`), before any bytes are
emitted, mirroring `maybe_field`'s check at `field_writer.hpp:195`:

```cpp
    const auto& alternative = std::get<idx>(variant);
    if(!E::constraint_checker(alternative))
      return std::unexpected(error_reason::validation_failure);
    return write_field<E, F>(alternative, field_list).template write<endianness>(s);
```

Both return `error_reason::validation_failure` as an `rw_result`; the enclosing
cast step attaches `failed_at` and widens to `cast_error`. `struct_cast_impl`
wraps a failed read with `fields::field_id`, which for the alternative's failure
is the *union* field's id — exactly what 050's criterion asks for, with no
`cast_error` member added and no new enumerator.

Constraint type: the tag's `::type`. `constraint_of_pack<type, opts...>` resolves
against it, `constraint_option_like<O, type>` classifies against it, and
`constraint_checker` is invoked on `field.value` whose type is `field_type` =
`type`. All three agree by construction, which is the property that makes the
spec's "the constraint applies to `std::vector<T>`, not `T`" true rather than
merely stated.

**No `if constexpr` guard on the check.** 050 asks that an alternative declaring
no constraint "is not charged for a check it did not ask for". `no_constraint<T>`
is an empty functor returning a literal `true`, and the struct-level fold already
calls exactly that, unconditionally, for every ordinary field in the library.
Adding a guard here — `if constexpr(!no_variance_field_like<T>)` would express it
in one line, since that trait means precisely "this field's constraint is
`no_constraint`" — would make the union path the only place in the codebase that
special-cases the no-op, for no measurable gain and one more thing to keep in
sync. Recorded as a decision rather than an omission; the one-line form is there
if the criterion is meant strictly. (Aside: `no_variance_field` is a poor name
for "carries no constraint" and is a candidate for the deferred naming sweep on
this branch.)

---

## 7. Slice 051 — the bound reaches the read path for free

**Decision: no read-path change whatsoever. Threading is already done by 048.**

`read_field`'s two allocating specializations read the ceiling off the field
type, not off a parameter:

- `field_reader.hpp:50` — `read_impl<endianness, bound_in_bytes<T::field_bound>>(…)`
- `field_reader.hpp:157` — `checked_byte_count<…, bound_in_bytes<T::field_bound>>(len_to_read)`

`read_variant_impl` constructs `T field;` — the choice field — and hands it to
`read_field<T, F>`. Once `to_field_choices` passes `tag::bound` as `field`'s
fifth argument (048) and `tag::bound` starts resolving from the pack (051), the
alternative's ceiling arrives at both sites with no new plumbing. The choice field
built from `as_vec` is `variable_sized_field_like` (its type is
`variable_sized_buffer_like` and its size `variable_size_like`) and the one built
from `as_vec_of_records` is `vector_of_record_field_like`, so each lands on the
specialization that reads `field_bound`.

That is the whole of 051's read path. What 051 actually contains is: widening
three tags' concept from `field_option_like` to `boundable_field_option_like`,
switching `::bound` from `use_default_bound` to `bound_of_pack`, the negative
cases, the tests, and deleting the now-false caveat paragraph at
`docs/reading.md:180-183` ("a `vec` alternative inside a `variance` cannot
declare its own ceiling"), which `dev/design/unbounded-resize-from-wire-length.md`
§8.1 recorded as a known gap.

Bound-on-a-meaningless-tag rejection needs no new concept either. The relationship
"a bound is meaningful only beside a variable size" is already hoisted into
*which template accepts it*, exactly as 047 hoisted it into which descriptor
accepts it: the three container tags carry `requires variable_size_like<…>`, so a
fixed size is unspellable inside them and accepting a bound is unconditionally
correct; the other five have sizes fixed by the tag itself, so a bound there is
unconditionally wrong and fails `constraint_option_like`/`field_option_like` as
an unclassifiable entry. `bound_pack_option_like` is reused verbatim.

Negative cases, each verified against a no-`CASE` control build per 051's
criterion: `rejects_bound_on_as_struct`, `rejects_bound_on_as_trivial`,
`rejects_bound_on_as_fixed_arr`, `rejects_duplicate_bound_on_tag`. They go in
`test/must_not_compile/option_pack_misuse.cpp` as further `CASE`s, because they
are the same subject the file already documents.

---

## 8. Slice 052 — the union-level constraint

```cpp
// field.hpp
template <fixed_string id, typename type_deducer,
          auto constraint_on_variant = no_constraint<typename type_deducer::variant>{}>
struct union_field : field<id, typename type_deducer::variant,
                           size_dont_care, constraint_on_variant> { … };

// field_descriptors.hpp
template <fixed_string id, type_deduction_like type_deducer,
          constraint_option_like<typename type_deducer::variant> auto... opts>
  requires (has_unique_field_choices(extract_field_choices<type_deducer>::value)) &&
           (has_unique_match_values(extract_match_values<type_deducer>::value))
using variance =
  union_field<id, type_deducer,
              constraint_of_pack<typename type_deducer::variant, opts...>>;
```

The default argument may name `type_deducer` because it is an earlier parameter,
so every existing `union_field<id, guide>` construction keeps its current
meaning. Enforcement needs **no new call site**: `union_field` derives from
`field`, so `fields::constraint_checker` in the struct-level fold is the union's
own constraint, and `is_derived_target_v` is false for a union field so the write
side's guarded check runs too. Finding 3 re-verified at both fold sites.

Blast radius — eight partial specializations pattern-matching
`union_field<id, type_deducer>`, each a one-parameter mechanical edit, all of
which ignore the new parameter (none needs to forward it, unlike
`to_optional_field` in 047):

| File | Sites |
|---|---|
| `field/field_traits.hpp` | `is_union_field` (:150) |
| `field_list/field_list_metadata.hpp` | `extract_length_dependencies` (:113), `extract_type_deduction_dependencies` ×3 (:160, :171, :209), `extract_switch_discriminants` (:257) |
| `field_write/derived_value.hpp` | `discriminant_obligation` (:55), `union_len_obligation_of` (:141) |

Same shape and same failure mode as 047's 17 sites: a missed one drops the union
out of a classification trait and fails loudly at the first schema that uses it.

`variance`'s pack admits **only** a constraint, per 052's criterion — a size or
bound entry fails the per-element `constraint_option_like`. That is correct
without argument: a union's own size is `size_dont_care` and it drives no
allocation of its own, only its alternatives do.

052 is independent of 048–051 and touches no file 049 migrates, so it can be
pulled forward if a short slice is wanted. Recommended order remains
048 → 049 → 050 → 051 → 052, with 050/051 interchangeable, because 052's tests
want a per-alternative constraint present to test the interaction 052's criteria
call out as most likely to be got wrong.

---

## 9. Decisions, stated explicitly

**State and data lifecycle.** Nothing persistent, and one ordering invariant that
matters. The per-alternative bound preserves 047's "validate, then allocate"
boundary unchanged — it reaches `checked_byte_count` at the same point above the
`resize` and above the constexpr/runtime branch, so a rejected alternative leaves
the choice field's container untouched. The per-alternative *constraint* sits on
the other side of that boundary by necessity: it can only run after the value is
read, so on the read path a failing alternative has already allocated and filled
its container. That is observable only as a discarded temporary — `read_variant_impl`
returns before `variant = std::move(field.value)`, so the union's value is never
partially assigned, and the enclosing `struct_field_list` is discarded by the
caller as with any other read failure. On the write path the check runs *before*
any bytes reach the stream, which is the asymmetry that matters: a partially
written stream is not recoverable, a discarded temporary is. Nothing here is
reversible, replayable or auditable, and nothing needs to be. Everything else in
this design is consumed during translation.

**Error propagation.** Unchanged in mechanism, and deliberately so. Two seams,
both pre-existing. `read_variant_impl`/`write_variant_impl` speak `rw_result` and
raise `error_reason::validation_failure` — the same enumerator the fold raises
for an ordinary field's constraint and the same one `maybe_field` raises for an
engaged optional's. `struct_cast_impl`/`stream_cast_impl` attach `failed_at` and
widen to `cast_error` at the layer that already does that for every other reason.
No layer learns anything new about the one below it: the variant machinery knows
a value and a predicate and does not know the field's name; the cast step
supplies it. The deliberate loss is *which* alternative failed — the spec fixes
`cast_error`'s shape, so a failure inside alternative 2 of field `"body"` is
reported as `{validation_failure, "body"}`, indistinguishable from a failure of
the union's own constraint. Recorded as accepted, not overlooked: the union field
id is what a caller can act on, and the alternative index is recoverable from the
discriminant the caller already has. The bound path reuses
`error_reason::excessive_length` verbatim; 051 introduces no enumerator.

**Concurrency and ownership.** None arises, and the statement is the point.
Everything added is either a compile-time entity or a stack local: the choice
field `T field;` inside `read_variant_impl` is owned by that call and dies with
it; `write_variant_impl` holds a `const&` into a caller-owned variant for the
duration of one write. No statics, no thread-locals, no caching, nothing shared
between alternatives. `struct_field_list` is not thread-safe today and this work
neither makes it so nor assumes it. The property that would break silently if a
future version cached a resolved option pack somewhere: there is nothing to
cache, because every resolved value is an NTTP baked into a type.

**Reuse.** Used rather than reimplemented: `resolved_options` / `pack_options` /
`*_in_pack` / `*_of_pack` in their entirety (the whole point of the header split
is that tags get the *same* scan, so a fifth option kind is added once);
`bound_pack_option_like` and its hoisting argument verbatim from 047;
`checked_byte_count` and `bound_in_bytes` untouched — 051 supplies a different
constant to a gate that already exists; `field`'s fifth parameter as the carrier,
so no new plumbing between the tag and the reader; `no_constraint<T>{}` as the
"declared nothing" value; `maybe_field`'s explicit-check pattern as the precedent
for both new call sites; `field_choice_list` as the choice carrier.
Introduced for reuse: `type_condition_list` is the pipeline's single transport
for per-alternative options, and the next option added to the tags reaches
`to_field_choices` by appearing on the tag — no third list, which is the
structural requirement the spec states. `field/field_options.hpp` is the place any
future consumer of the pack vocabulary includes, and its existence is what stops
the next such consumer from copying the resolvers.

**Extension points.** Two, both internal, plus one explicit non-extension.
(1) A new option kind: one concept, one `*_in_pack` scan, one `resolved_options`
member, one member on each admitting tag, one argument in `to_field_choices` —
five edits in three files, with the contract that a tag exposes one member per
option kind and `to_field_choices` reads them positionally into `field`. That is
strictly better than today's "add a parallel list", which is what the spec asks
for. (2) A new type tag: define it, give it the four members, add an
`is_type_tag` specialization. There is deliberately **no** extension point for a
user-supplied option kind or a user-registered tag — `size_option_like` and
`is_type_tag` are closed sets by explicit specialization, per
`dev/design/schema-api-verbosity.md`'s Non-Goals, and opening either would make
the pipeline's classification a protocol the library cannot verify.

**Build vs. buy.** Nothing here is buyable, and the one place it could have been
was already settled: Boost.Hana / Boost.MP11 for the pack scan was evaluated and
rejected in 045 (the project's own override mandates hand-rolled vocabulary types
in `include/lib/` specifically to keep template errors readable, and s2s having no
dependencies is a stated property). This design's reuse decision is the same
question one level down — buy the *project's own* scan rather than build a second
one — and the answer is the header split. `std::variant` is bought and stays
bought; `std::tuple` as an alternative carrier for the resolved options was
considered and rejected for the same reason 045 rejected it, that an `auto...`
pack is already the carrier and a tuple adds a type to diagnose through.

**Abstractions introduced.** Two, each with its forcing problem.
`type_condition_list<cases...>` — `type`'s three specializations must forward the
case pack to `to_field_choices`, a member alias cannot be a pack, and forwarding
the guide itself would put `switch.hpp`/`ladder.hpp` in `field.hpp`'s include
closure. `field/field_options.hpp` — not a new abstraction so much as a new
*location*, forced by a genuine five-header include cycle; it introduces no type
and moves the existing vocabulary unchanged.
Explicitly **not** introduced: no per-tag "option policy" trait (each tag's own
parameter list carries its concept, which is where the information already
lives); no base class or CRTP for tags (four static members, and `is_type_tag`
does the classifying); no wrapper type for a bounded or constrained alternative
(the options live in the tag, and a wrapper would interact combinatorially with
nothing while adding a kind every trait must see through); no `if constexpr`
elision of the no-op constraint check (§6.2).

**Alternatives rejected** — collected; each argued in place.
Passing `type_switch`/`type_if_else` through `to_field_choices` directly (§1);
`using conditions = type_switch;` via the injected-class-name (§1); constraining
`to_field_choices`'s pack with `type_condition_like` (§1); leaving the pack
machinery in `field_descriptors.hpp` and duplicating the resolvers for tags (§2);
relocating `type_tags.hpp` to break the cycle instead (§2); inheriting tags from
`resolved_options` (§3); dropping `as_trivial`'s `fixed_size_like` conjunct to
match `basic_field` exactly (§3); giving the sizeless tags an admits-nothing pack
in 049 (§6.1); guarding the per-alternative constraint check with
`if constexpr` (§6.2); a per-alternative bound threaded as an explicit argument
through `read_variant_impl` rather than through `field_bound` (§7 — rejected
because it would be a second path to a ceiling the field type already carries,
and two paths to one ceiling is how they drift); a `union_field` wrapper type for
the union-level constraint instead of a third parameter (§8 — rejected on the
same grounds 047 rejected `bounded_field`: a new field kind every trait must see
through, for no ordering benefit).

---

## 10. Tests

Routing is by constant-evaluability and the compiler enforces it
(`dev/specs/compile-time-test-tier.md`). Everything below that touches a stream
gets both a `ut` source under the `add_ut_test` triple build and a GoogleTest
source under `add_struct_cast_test`. Test lambdas must not capture — constraint
and bound values belong in the schema type or as `constexpr` locals inside the
lambda, never captured from the enclosing scope, or `*_coverage` catches it and
the other two entries stay green.

**048** — no test edits, by criterion. Two *additions*, owed by §3's
`is_type_tag` fix: a round-trip over an `as_arr_of_records` alternative and one
over an `as_vec_of_records` alternative, in `union_read_ct` / `union_write_ct`
and their GoogleTest mirrors. These are the first coverage those tags have ever
had.

**Added during scaffolding: `test/internals/union_choice_pipeline_ct.cpp`.**
048's stated criterion is negative — "the existing suite passes unchanged" —
and a negative criterion cannot distinguish *builds the same fields* from
*builds different fields that happen to break no existing assertion*. The
positive form is a `static_assert` that `variance<...>::field_choices` equals a
spelled-out `field_choice_list`, alternative by alternative, with each one's
id, type, size, constraint and bound. Written and passing **before** 048, so it
is a description now and a regression gate afterwards; verified non-vacuous by
corrupting one expected size and confirming the compile-time build fails. It
belongs under `internals/` rather than `schema/` because it asserts a type
rather than a round-trip, and it is where 050 and 051 should assert that a
declared constraint and bound land in `field`'s fourth and fifth arguments —
visible there without a stream, and for 051 that assertion *is* §7's claim that
the bound needs no read-path plumbing.

**049** — `rejects_duplicate_size_on_tag`, `rejects_unsized_as_string`,
`rejects_oversized_as_trivial` in `test/must_not_compile/option_pack_misuse.cpp`.
Plus a positive `ut` case that the same tag spelled with its size in a
one-entry pack still round-trips, which is the whole claim of the slice.

**050** — a new pair, `test/schema/union_alternative_options_read.cpp` +
`_ct.cpp` and `..._write.cpp` + `_ct.cpp`, rather than additions to
`union_read`/`union_write`: the tree is organised by what a file verifies, and
this verifies an option on an alternative, not the variance construct. Contents:
a constrained alternative accepted; the same rejected with
`validation_failure` and `failed_at == "<union id>"`; **and the non-vacuity pair
050 insists on** — a read-side case that fails if the `read_variant_impl` check
is deleted and a write-side case that fails if the `write_variant_impl` check is
deleted, as two separate tests, since one test covering both paths passes with
either call site missing. Verify by deleting each check locally and confirming
exactly one test goes red. Plus a constrained alternative composed with a size
entry in both orders, and `rejects_duplicate_constraint_on_tag`.

**051** — extends the 050 pair (same subject: an option on an alternative) with
below-bound / at-bound / over-bound for `as_vec`, `as_string` and
`as_vec_of_records`, asserting `excessive_length`; one case where the bound is
not a multiple of `sizeof(element)`, since the gate is an integer division; and
one proving `S2S_DEFAULT_MAX_BYTES` does not clamp a declared alternative bound
in either direction — the existing `allocation_bound_default_override_ct` /
`allocation_bound_disabled_ct` are the models. Negative cases per §7, each
checked against a no-`CASE` control build.

**052** — a `variance` carrying a constraint, read and write, both tiers; the
non-vacuity proof; a size entry and a bound entry on `variance` as
`must_not_compile` cases; and the interaction case 052 names as most likely to be
got wrong — a union-level constraint *and* a per-alternative constraint both
declared, asserting each fires independently.

`single_header/s2s.hpp` is regenerated in every one of the five commits;
`test/must_not_compile/` and `test/shipped_header/` consume the amalgam, so a
stale one silently tests the previous tree.

### Scaffolding, and the one rule it follows

Landed ahead of implementation: `test/internals/union_choice_pipeline_ct.cpp`
(above), the `union_alternative_options_{read,write}{,_ct}.cpp` quartet
registered in `test/schema/CMakeLists.txt`, and CASE 5-10 appended to
`test/must_not_compile/option_pack_misuse.cpp`.

The quartet's placeholders are not trivial passes. Each is a real assertion
that an alternative carrying *no* options reads and writes correctly — the
baseline every option case contrasts against — so a regression in plain
variance handling surfaces there instead of being misread as an option bug.
Read and write are separate files rather than one round-trip precisely because
050's two constraint call sites must be shown to fail independently, and a
round-trip stays green with either missing.

**CASE 5-10 are deliberately not registered.** `add_rejected_case` sets
`WILL_FAIL`, so a case failing for the wrong reason passes the harness while
testing nothing — the failure this suite actually hit on CASE 4 during 047. All
six currently fail because the tags take no pack at all, which is not the
diagnostic any of them exists to prove. Each gets registered by the slice that
implements it, after a no-`CASE` control build confirms the file still compiles
clean. That control was run at scaffolding time and passes.

---

## 11. Documentation

`docs/schema/optional-and-variant.md` is the owner; the tag table at :131-140 is
what `test/schema/union_*` mirrors.

- **049** — table rows become `as_trivial<T, opts...>`, `as_vec<T, opts...>`,
  `as_string<opts...>`, `as_vec_of_records<T, opts...>`, with a sentence that
  entries are order-independent and that a tag's options are the same options its
  descriptor takes. Note `as_trivial<T>` defaulting to `sizeof(T)`.
- **050** — a worked constrained alternative, and the sentence the spec asks be
  stated plainly: the constraint applies to the tag's resulting type
  (`std::vector<T>`), not its element type.
- **051** — bounding an alternative, pointing at `docs/reading.md`'s allocation
  limits section; **delete** the now-false caveat paragraph at
  `docs/reading.md:180-183`.
- **052** — the two constraints distinguished: one constrains the resolved
  variant, the other a single alternative's payload, and both apply when both are
  declared.
- `docs/schema/size-axis.md:79-85` (`size_choices` is not declarable) stays true
  and stays as-is; 048 removes its last internal consumer but does not make it
  declarable, and 027 remains open.

Any complete program shown needs a `test/doc_examples/` source with the
`<!-- docs: … -->` / `// docs-begin` binding; prefer extending
`guide_variance_example.cpp` over adding a target.

---

## 12. Findings and risks

1. **`as_arr_of_records` and `as_vec_of_records` are documented but unusable
   today** — no `is_type_tag` specialization, so `type_tag_like` is false and
   `match_case`/`branch` reject them. Fixed in 048 (§3), with a stated deviation
   from that slice's inertness.

   **Resolved during 048, and the round-trip found one of the two broken.**
   Reading works for both tags; writing works for `as_arr_of_records`. Writing
   an `as_vec_of_records` alternative **fails**, with
   `found_contradicting_length` at the length field.

   The obligation machinery has the agreement half but not the derivation half.
   `union_len_obligation::agrees` (`field_write/derived_value.hpp`) compares the
   held vector's `size()` against the length field's value, but nothing derives
   that value from a union-held container the way it is derived for a plain
   `vec_field`. The length stays 0, `agrees` correctly reports the
   disagreement, and the write is rejected. Because the length field is a
   length target, 043 makes it non-assignable — so there is no spelling of the
   schema that works around it.

   Pre-existing and previously unreachable: the tag could not be named, so this
   path had never run. Not widened into 048, per this section's own rule. Needs
   its own issue. `union_read_ct.cpp` covers both reads and
   `union_write_ct.cpp` covers the array write; the vector write is documented
   in place as absent, with the diagnosis, rather than left as a silent gap.
2. **`basic_field` lacks the `fixed_size_like` conjunct** that `as_trivial`
   keeps (§3). `basic_field<"x", u32, len_from_field<"n">>` relies on SFINAE
   through an incomplete `deduce_field_size` primary to produce a diagnostic.
   Out of scope; worth its own issue.
3. **A `variance` with two alternatives of the same C++ type is still broken**,
   as `dev/design/schema-api-verbosity.md` §8 recorded:
   `read_variant_impl`'s `variant = std::move(field.value)` is a converting
   assignment with no unique best alternative for `std::variant<u32, u32>`. This
   design adds the per-alternative constraint check on the line immediately
   above that assignment and does not fix it. Unchanged and still deserving its
   own issue — and note that `has_unique_field_choices` rejects such a schema at
   the `variance` alias anyway, so the reader bug is reachable only through
   `union_field` directly.
4. **MSVC 19.39 remains unverified**, as in both prior designs. The construct
   most at risk here is the partial specialization
   `to_field_choices<id, type_condition_list<cases...>>` reaching
   `cases::type_tag::constraint` as a class-type NTTP argument to `field`. It is
   the same shape 048 replaces (`to_field_choices` already deduces `auto...
   sizes` out of `size_choices_t` and feeds them to `field`), so the risk is low,
   but there is no build CI to catch it.
5. **The `no_variance_field` name** means "carries no constraint" and is used to
   gate `maybe`'s base field; it is the natural trait for §6.2's rejected
   `if constexpr` and reads wrong in both places. Candidate for the deferred
   naming sweep already tracked on this branch.
6. **Almost nothing in this design was compiled — with one exception, since
   resolved.** Every claim about include direction, consumer counts and
   pattern-match sites was verified by reading the tree rather than by building.
   The one construct this section originally flagged for a prototype —
   that `to_field_choices<id, type_condition_list<cases...>>` instantiates with
   `cases::type_tag::constraint` as `field`'s fourth argument — **has been
   prototyped and holds**, clean on g++-14 and clang++-19 under
   `-Wall -Wextra -Wpedantic`:

   - `std::is_same_v<built, expected>` passes for a three-alternative guide
     mixing a constrained `as_trivial`, a bounded `as_vec`, and a bare `as_vec`
     taking both defaults.
   - The constraint survives as a *callable*, not merely as a template
     argument: `first::constraint_checker(50u)` and
     `!first::constraint_checker(200u)` are both compile-time true. This is the
     property §6.2's call sites depend on, and it is the one a type-level
     `is_same` check alone would not have caught.

   Why the risk was lower than it looked: the new shape deduces a pack of case
   *types* and never asks the compiler to deduce a value at all — the class-type
   values are read off the tag by name and forwarded. That is the easy
   direction. The rule that constrained 044 bites when a partial specialization
   tries to match *inside* a class-type NTTP, which nothing here does.

   Finding 4's MSVC caveat is unaffected: two compilers agreeing is not three,
   and there is still no build CI.
7. **The constraint spelling this design and 050 both use does not compile.**
   `s2s::range{1u, 99u}`, as 050's issue writes it, is issue 026: `range`'s
   constructor applies `static_assert` to its own function parameters. Found
   during 050, which uses `lte` instead. It bites 052 the same way, and the
   more general form of it is that a constraint over a container has to be a
   functor — `eq{std::vector<u8>{…}}` is not spellable as a template argument,
   because `std::vector` is not a structural type. Every container-level
   constraint in the tests and docs is therefore an empty struct with a
   `constexpr operator()`.
