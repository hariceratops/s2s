# A schema cannot be a plain struct

Brief, from a design conversation on 2026-11-07. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

A schema today is a type assembled from a descriptor DSL:

```cpp
s2s::struct_field_list<
  s2s::basic_field<"sample_count", u32, 4_B>,
  s2s::vec_field<"samples", u16, s2s::len_from_field<"sample_count">>
>
```

The parsed value is that type, read through `frame["samples"_f]`. C++26 static
reflection (P2996) plus annotations (P3394) would let the same schema also be a
plain struct with annotated members.

The developer has settled the shape: "2 frontends for users but one backend
inthe lib".

- `struct_field_list` stays the primary front end at every language tier.
- The annotated struct is a second front end in the C++26 tier, behind a
  preprocessor switch (see `a-cxx20-project-cannot-include-s2s.md`).
- Both lower into one backend, "as much value based as possible".

The developer's request covers two jobs:

- "cleanups for reflection and general. we have many dead code", which is v1
  work;
- "refactoring for constexpr first code so that it maps to reflection +
  annotation later", which is mostly not.

v1, due January 2027, is s2s feature-complete in C++23: bitfields, the
lazy-records generator, and the cleanup. Its refactoring has a stated limit:
"refactoring stops with benchmarks, performance tuning, namesweep and dead code
removal. if needed the bareminimum changes to introduced as much value based
metaprogramming". The "if needed" is now defined by the v1 rule below.

Benchmarks and copy removal have their own brief
(`a-parsed-record-is-copied-on-its-way-out.md`); in v1, storage is passed down to
nested readers. The shared value-based backend, the annotated front end and the
`std::meta::info` swap come after v1. January is a code deadline. The Meeting
C++ talk comes later, and its content is "largely undecided".

## Considerations the discussion surfaced

**What is settled.**

- **Backend:** one backend, as value-based as possible.
- **The annotated front end parses in place,** directly into the user's members,
  with no runtime copy. That is my reading of "annotate forms copy into the user
  members, copy at runtime is a cost and we need zero cost abstraction". It could
  also mean a compile-time-generated copy, so this needs confirming.
- **`type_id` becomes `std::meta::info` in the C++26 tier:** "we adapt the s2s
  entities to fit not the other way around".
- **v1's refactoring:** benchmarks, tuning, the naming sweep and dead code
  removal. Any value-based change follows the v1 rule below, which is decided.
- **The naming sweep in v1 covers plainly-wrong names:** "things which are
  obvious wrong concept names type traits, functions which do not express
  intent".
- **`range` constraints and `size_choices`:** "delete if unused". Both are
  unused, so both are deleted in v1.
- **Cleanup is hierarchical:** "refactoring is hierarchial if a parent is removed
  so is the child".
- **`is_size_like` is decided by a check at deletion time:** "is_size_like can be
  delete if and only if there are no other users once you delete other
  neighbours" (below).
- **A failed read, for any API that returns `expected`:** the caller sees a
  complete record or a `cast_error`, as today (copy brief). Storage the caller
  keeps is post-v1 (below).
- **The TMP-to-reflection notes come into `dev/`.**

**The v1 rule for value-based changes (decided).** The codebase uses two styles:

- **Type-based:** a schema is a type pack, and metadata comes from templates
  recursing over it. `extract_length_dependencies` has seven partial
  specialisations (`field_list_metadata.hpp:39-124`).
- **Value-based:** the same metadata is held as constexpr values, computed by
  ordinary loops. `record_layout`'s width and offset arrays
  (`fixed_layout.hpp:184-213`) already work this way.

In v1, value-based metaprogramming is used in exactly three cases:

1. **New code that computes data is written as `constexpr` functions over
   values.** This covers any new v1 code that produces a number, a name, a list
   or a table: a mask, a shift, a width sum, a dependency list, an offset.
2. **Existing code is converted only where it is already open for v1 work.**
   That means the one trait or table being edited, and nothing next to it.
3. **Existing code is converted where a benchmark shows it is the problem.** The
   table capacities (`max_union_choices = 8`, `deps[64]`) are the likely case,
   if the GIF schema hits them.

**Code that chooses a type stays as templates.** That covers which reader handles
a field kind, and what C++ type a field has. Before C++26 a type cannot be a
value.

**`constexpr`, not `consteval`.** The ut tier builds every test three times, one
of them under `UT_RUN_TIME_ONLY`. A `consteval` function cannot be called at run
time, so its tests could cover only the compile-time build. A `constexpr`
function called in a constant expression still runs at compile time.

Everything outside the three cases goes onto the tier-bound inventory for the
post-v1 backend. The test for any proposed change: is it new data code, code
already open, or a measured problem? If none of these, it waits.

How the rule applies to the v1 work:

| v1 work | Case | What changes |
|---|---|---|
| Bitfields | 1 | Masks, shifts and the width checks become value-based. |
| Bitfields, bit members as lengths or terminators | 2 | `extract_length_dependencies` is opened. |
| Passing storage down | — | Nothing. It changes who owns storage; dispatch still selects types. |
| The generator, the benchmarks themselves, the deletions, the naming sweep | — | Nothing. The benchmarks are case 3's trigger. |

**The cleanup has a second purpose.** The developer wants it "to have realistic
expectation of what has to be ported both directions". So v1's cleanup should
leave an inventory of what is tier-bound. That inventory also collects whatever
the v1 rule leaves for later.

**Hierarchical deletion needs a stopping rule.** "If a parent is removed so is
the child" works for a child with one parent. A child with another parent stays,
and loses only the part that served the deleted parent. Whether a child still has
other users is checked when the deletion happens, not assumed in advance.

Deleting `size_choices` shows how this plays out.

- **Goes.** Each item exists only for the one above it:
  - `size_choices_t` and `size_choices` (`field_size.hpp:83,115`);
  - `num_of_choices`;
  - `is_selectable_size`, primary and specialisation (`field_size.hpp:280-288`);
  - `is_selectable_size_v` (291);
  - `selectable_size_like` (294);
  - `atomic_size` (265), whose only user is the specialisation's requires-clause
    (285).
- **Stays, minus a disjunct.** `size_option_like` (`field_options.hpp:45-50`)
  loses its `selectable_size_like` line. It has other users, so it stays.
- **Checked at deletion time: `is_size_like`** (`field_size.hpp:297-301`). It
  also loses its `selectable_size_like` line. The developer's rule is that it is
  deleted "if and only if there are no other users once you delete other
  neighbours". So once `size_choices` and its dependents are gone, look for
  remaining users. Delete it if there are none; otherwise keep it with the
  shorter disjunction.

  As of 2026-11-08, a grep of `include/`, `test/`, `docs/` and `examples/` finds
  only its own definition. The check is still made at deletion time, against the
  tree as it is then. `todos.md` already lists it as a dead declaration.
- **Issue 027 has to follow the decision.** Its acceptance criteria name
  `is_size_like`, and its subject (`is_selectable_size` reading the wrong trait)
  disappears with `size_choices`. Deleting `is_size_like` therefore means
  amending or closing 027.
- **Docs.** The section at `docs/schema/size-axis.md:329-334` and the entry at
  `docs/reference.md:271-272`.

The range constraints follow the same pattern:

- **Goes.**
  - `range`, `is_in_open_range` and `is_in_closed_range`
    (`field_value_constraints.hpp:141-198`), and their deduction guides
    (222-226);
  - their `std::sort`/`std::lower_bound` calls;
  - `docs/constraints.md:46-59` and `docs/reference.md:273-274`.

  Issue 026 is closed by the deletion.
- **Stays.** `<algorithm>`, because `std::find` (line 134) and `std::copy_n`
  (`fixed_string.hpp:17`) still use it.

**The rest of the dead code, located.**

- `lib/memory/bit.hpp`: entirely commented out, included by nothing.
- `lib/containers/static_array.hpp`: included by nothing, tested by nothing.
- `meta::invoke` (`mp.hpp:52-78`): used only by `traits_ct.cpp:138-140`; its
  tests go with it.
- `typelist::any_of`, `front` and `null`: their tests (`traits_ct.cpp:88-91`) go
  too. Only `all_are_same_v` is live.
- `fixed_string_list.hpp`'s `size`, `front`, `get`, `pop` and three tag types:
  their tests (`traits_ct.cpp:69-78`) go too.
- `constexpr_read_trait`/`constexpr_write_trait` (`stream_traits.hpp:37-45`):
  the warning box at `docs/streams.md:164-171` goes with them.
- `constexpr_buffer_size` (`address_manip.hpp:63`).
- Unused includes: `<cassert>`, `<cstdio>`, `<cstring>`, and `<string>` in
  `typelist.hpp`.

**33 todo/status comments sit in `include/`, and some are already wrong.**

- `switch.hpp:13` and `ladder.hpp:14` say "possibly unused", yet `type.hpp` reads
  `variant`.
- `struct_cast.hpp:4` says "split to cast and cast impl", yet `include/cast/`
  exists.
- `sentinel_terminator.hpp:90` dates formatted `static_assert` messages to C++23.
  I believe they are C++26 (P2741).

**Some live code is in the wrong place.**

- `computation_from_fields_impl.hpp:11` has `using namespace s2s_literals;` at
  global scope in a header.
- `algorithms.hpp` declares five functions in the global namespace.
- `std::string_view{fields::field_id.data()}` is written out five times, right
  beside `as_sv`.
- `-Wnon-template-friend` fires from `mp.hpp:16`.

**What the v1 naming sweep covers: plainly-wrong names.**

- From `todos.md`: `is_size_like` (if it survives the deletion-time check), the
  forced `compute_t`/`compute` split, `stream_wrapper_impl`, and
  `derived_value.hpp`.
- `field_type_info.hpp:6`, which carries its own "todo better name".
- `extract_type_from_array_v` (`type_traits.hpp:248`), a type alias spelled
  `_v`.

The sweep runs last in v1, after bitfields, the generator and the deletions.

**How far "as value-based as possible" reaches depends on the tier.** At every
tier, a constexpr array of field descriptors can hold names, kinds, sizes, length
sources, bounds and dependency lists. Below C++26 it cannot hold:

- a field's C++ type, except as a `type_id`;
- a constraint;
- a computed-length callable.

Those stay as type-level slots keyed by field index. In C++26,
`std::meta::info` and `reflect_constant` can carry them (the name should be
checked against P2996R13). The central post-v1 problem is how one backend reads
both forms without `#if` inside functions (`AGENTS.md`).

**v1 builds half of the storage seam.** To parse in place, the backend needs two
things from either front end:

- "the storage of field `i`";
- "the value of the field named `n`", which is `field_value_of` today
  (`field_list.hpp:112-128`).

Passing storage down gives the DSL front end the first. The annotated front end
later supplies `obj.[:member:]`. What v1 does not build is storage for a length
target that is not a member of the user's struct.

**A failed read into storage the caller keeps is a post-v1 contract to choose.**
For an API that returns `expected`, failure is settled in v1: the in-place return
assigns the error over the partly filled record, which destroys it.

That stops holding for an API that writes into storage the caller keeps: a
future `struct_cast_into(dest)`, or the annotated front end with the user's own
struct. There are three possible contracts:

1. **Contents unspecified after a failure.** This costs nothing, and matches
   iostream's `operator>>`.
2. **Reset to default on failure.** This costs one reset on the failure path.
3. **Untouched on failure.** This needs a read into a temporary first, which
   brings the copy back.

The coordinator recommends 1, or 2 if callers need a known state. It is not
decided. Whichever is chosen applies to both front ends.

**`type_id` becomes `std::meta::info` in C++26, and s2s's entities adapt.**
`type_id` is load-bearing: there are 3 `type_id` sites and 14 `type_of` sites
across 6 headers, and it is the first template argument of every record. It stays
below C++26, so v1 ships with:

- a mechanism the committee has long said it means to make ill-formed (CWG 2118;
  status not checked);
- the warning above;
- what reads as a ceiling of about 1024 types per translation unit
  (`mp.hpp:23-38`, inferred).

**What the annotated front end needs from annotations.** Still unanswered. These
have no direct mapping:

- length targets, which the DSL hides (`field_list.hpp:20-23`);
- discriminants and frozen fields, which are const through `operator[]`
  (`field_list.hpp:74-91`). On write, is a disagreeing value overwritten or
  rejected?
- names given as strings;
- unions and optionals, which carry types or callables.

The backend already derives lengths and discriminants (`len_obligation` and
`discriminant_obligation` in `field_write/derived_value.hpp`), so hiding them is
front-end policy.

**Why the DSL couldn't just be reflected before.** The schema "is not a C++
aggregate", and existing reflection libraries cannot "express derived lengths or
discriminant inversion" (`dev/design/struct-to-stream-write.md:684-689`). A front
end over s2s's own backend answers both points.

**Bitfields come first, and the v1 rule makes their masks value-based.** Masks,
shifts, the widths-must-sum check and the type-wide-enough check are data, so
they come from a constexpr array of widths (case 1). The `bit_field<...>`
spelling and the reader dispatch stay type-level.

Two benchmark formats ask for more. IPv4's IHL and CAN's length code are bit
members that act as length sources, and EtherCAT's M bit ends a sequence. Both
need the dependency tables to see inside a container, which opens
`extract_length_dependencies` (case 2). Under C++26, `bit_size_of` supplies the
widths array directly.

**The generator wants nothing from the descriptor.** It is v1, on
`std::generator`, and yields by value. The `AGENTS.md` "constexpr by default"
override has to name it as an exception.

**Compiler status, checked 2026-11-07.**

- **GCC 16:** P2996R13 under `-std=c++26 -freflection`, with partial P3394R4.
- **Upstream Clang:** none. Only the Bloomberg fork has it.
- **MSVC:** nothing public.

**The talk and the code may drift.** A GCC-16-only spike is post-v1, and whether
to do one is a talk decision.

**What this means for January.**

- **v1:**
  - dead code, deleted hierarchically, with users checked at deletion time;
  - the `range` constraints and `size_choices`, with their cascades, docs, and
    issues 026 and 027 (and `is_size_like` if the check finds no users);
  - stale todos, include hygiene, and code in the wrong place;
  - the tier-bound inventory;
  - passing storage down, with the in-place return at the top;
  - value-based changes under the v1 rule;
  - the plainly-wrong-names sweep, last.
- **Post-v1:**
  - the value-based backend;
  - the annotated front end and its annotations;
  - the failure contract for caller-kept storage;
  - `type_id` → `std::meta::info`;
  - the spike, if the talk wants it.

## Directions rejected

**Replacing `struct_field_list` with reflection.** C++26 annotations are largely
unsupported, and version-locked projects need a working s2s.

**Today's descriptor types as the shared backend form.** Set aside by "as much
value based as possible".

**Parsing into `struct_field_list_impl` and copying.** The developer rejects any
runtime copy.

**Adapting `std::meta` to s2s's entities.** The entities adapt instead.

**A full rename in v1.**

**Fixing `range` constraints and `size_choices`.** They are unused, so they are
deleted.

**Deciding `is_size_like` in advance.** It is checked for users at deletion time.

**Leaving caller-kept storage untouched on failure by reading into a temporary
first.** It brings back the copy being removed.

**Value-based "only when a v1 feature can't be built cleanly without it".** Too
strict. Almost anything can be built type-level, so bitfields' masks would be
written as templates and then rebuilt after v1, even though values cost nothing
extra now.

**Value-based "also when it saves rebuilding a v1 feature after v1".** It leaks.
The post-v1 backend replaces every type-level piece, so the reasoning holds for
the `read_field` dispatch, `field_traits` and every `extract_*` specialisation,
and would justify the whole backend in v1. It also depends on a post-v1 design
that doesn't exist yet.

**Value-based "also when it is simply clearer".** It is taste, and can't settle a
disagreement.

**`consteval` for the value-based functions.** The ut tier's run-time build
can't call them.

## Questions still open

- Post-v1: for storage the caller keeps (`struct_cast_into(dest)`, the annotated
  front end), are the contents unspecified after a failure, or reset to default?
- Does "annotate forms copy into the user members" mean in place, as recorded, or
  a compile-time-generated copy?
- Which annotations does the second front end need for length targets,
  discriminants and frozen fields? On write, is a disagreeing value overwritten
  or rejected?
- Where does a length target with no member live during an in-place parse?
- How does one backend read an IR whose type-level slots shrink at C++26, without
  `#if` inside functions?
- What does the talk show? Is a GCC-16-only spike wanted once v1 ships?
- Where are the TMP-to-reflection notes, and where in `dev/` do they land?
