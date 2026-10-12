# Design: reading two fields costs a whole record (struct views)

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`, which is frozen.
This design consumes it and does not revise it. The spec leaves six things
to the design. All six are decided here:

- entry-point and type names (§1)
- concept and sub-concept names (§2)
- the offset mechanism (§3)
- the sub-view's exact type (§1, §4.4)
- what `failed_at` holds for a short-span `buffer_exhaustion` (§5.1)
- the UBSan mechanism (§6)

§12 lists where the spec or the issues are wrong, underspecified, or blocked
by something already in the tree. Two of those findings need a decision from
the user before 075's tests are written: F1 (narrow foreign-order placement)
and F4 (clang cannot build the tree today).

Issues served: `dev/issues/074-ubsan-test-configuration.md` through
`dev/issues/082-document-views.md`. §10 maps each decision to the issues it
affects. §11 gives the slice order, which is 074→082 unchanged, with the
dependency notes that order hides.

Grounded in, and reusing rather than paralleling:

- `include/field_read/read_impl.hpp` (`read_impl`, `read_native_impl`, `read_foreign_scalar`, `read_foreign_buffer`)
- `include/cast/struct_cast_impl.hpp` (the per-field fold, and how it names a failure)
- `include/field_read/field_reader.hpp` (how `struct_field` and `array_of_records` strip an inner name)
- `include/field_validation/field_value_constraints.hpp` (`constraint_checker`, `is_eq_constraint`)
- `include/field_size/field_size.hpp` and `comptime_field_size_deduce.hpp` (`byte_count`, `deduce_field_size`)
- `include/field_list/field_list.hpp` and `field_list_metadata.hpp` (`lookup_field`, `field_type_info::occurs_at_idx`, the `operator[]` pattern)
- `include/field_list/announcing_record.hpp` (`census_of_list_v`, and `fixed_order_check` as the precedent §2.4 diverges from)
- `include/field/field_traits.hpp` (the field-kind traits)
- `include/api/struct_cast.hpp` (entry-point shape)
- the test helpers in `test/CMakeLists.txt` and `test/must_not_compile/CMakeLists.txt`

Several claims below were checked against gcc 14.2 and clang 19 on this
machine rather than assumed. They are marked **(verified)**.

---

## 0. The shape in one page

| Piece | Where | Slice |
|---|---|---|
| `is_no_constraint` / `is_no_constraint_v` | `field_validation/field_value_constraints.hpp`, beside `is_eq_constraint` and shaped like it | 075 |
| `record_layout<S>`: `widths`, `offsets`, `names`, `width` | `field_list/fixed_layout.hpp` (new) | 075; widened by 078, 079 |
| `wire_width<S>` (public constant) | `field_list/fixed_layout.hpp` | 075 |
| `every_field_has_compile_time_width` + atom `..._v` | `field_list/fixed_layout.hpp` | 075 (leaf narrowed, §2.5), final by 079 |
| `has_no_announcing_record`, `has_no_optional_field`, `has_no_variant_field`, each + atom `..._v` | `field_list/fixed_layout.hpp` | 080 |
| `fixed_layout_schema` (the composing concept) | `field_list/fixed_layout.hpp` | 075 (two conjuncts), 080 (five) |
| `any_field_constrained_v<S>` | `field_list/fixed_layout.hpp` | 077 |
| `raw_byte_like<B>`, `span_covers_record<extent, S>` + atom `span_covers_record_v` | `view/struct_view.hpp` (new) | 075 |
| `unchecked_byte_source<B>` | `view/struct_view.hpp` | 075 |
| `decode_field_at<F, order>(const B*) -> F` | `view/struct_view.hpp` | 075 |
| `view_read_t<F, order, B>` (accessor return type) | `view/struct_view.hpp` | 075; C-array arm 076; sub-view arm 078; array arm 079 |
| `struct_view<S, order, B>` (the view, and the sub-view) | `view/struct_view.hpp` | 075 |
| `array_of_records_view<S, N, order, B>` | `view/struct_view.hpp` | 079 |
| `struct_view_impl<S, order>` (creation) | `view/struct_view.hpp` | 075 (length), 077 (constraint walk), 078/079 (descent) |
| `struct_view_le`, `struct_view_be` | `api/struct_view.hpp` (new), shaped like `api/struct_cast.hpp` | 075 |
| `#include "api/struct_view.hpp"` | `include/s2s.hpp` | 075 |
| `fixed_string::to_sv` made a `const` member | `lib/containers/fixed_string.hpp` | 076 (§12 F8) |
| `S2S_SANITIZE_UNDEFINED` option | `CMakeLists.txt` | 074 |
| `run_time` CTest label on both helpers' run-time entries | `test/CMakeLists.txt` | 074 |
| `CMakePresets.json` with configure, build, test and workflow presets, all named `ubsan` | repo root (new) | 074 |
| `-Wno-non-template-friend` made GNU-only | `test/doc_examples/`, `test/shipped_header/`, `examples/` CMakeLists | 074 (§12 F4) |
| `build-ubsan/` | `.gitignore` | 074 |

**Deliberately not touched.** Each of these is load-bearing:

- **`read_impl.hpp`, `field_reader.hpp`, `struct_cast_impl.hpp`.** The view
  calls them; it does not fork them. That makes equivalence with the parser
  structural rather than tested-for (§4.1). It also means a future fix to the
  parser's byte placement (§12 F1) fixes views in the same commit.
- **`cast_error.hpp`.** The spec's Non-Goals forbid changes. That includes
  adding an `operator==`, so tests compare `failure_reason` and `failed_at`
  member by member (§12 F7).
- **`announcing_record.hpp`.** Only `census_of_list_v` is read from it.
  `fixed_order_schema` is deliberately *not* used on the view entry point
  (§2.4).
- **`stream_traits.hpp`.** No stream operation is added. The byte source in
  §4.1 satisfies the existing `identified_as_constexpr_stream`.

---

## 1. Public surface and names

```cpp
namespace s2s {
template <fixed_layout_schema S, raw_byte_like B, std::size_t extent>
  requires span_covers_record<extent, S>
[[nodiscard]] constexpr auto struct_view_le(std::span<B, extent> bytes)
  -> std::expected<struct_view<S, std::endian::little, std::remove_const_t<B>>, cast_error>;

template <fixed_layout_schema S, raw_byte_like B, std::size_t extent>
  requires span_covers_record<extent, S>
[[nodiscard]] constexpr auto struct_view_be(std::span<B, extent> bytes)
  -> std::expected<struct_view<S, std::endian::big, std::remove_const_t<B>>, cast_error>;

template <fixed_layout_schema S>
inline constexpr std::size_t wire_width = record_layout<S>::width;

// struct_view<struct_field_list_impl<metadata, fields...>, order, B>:
//   template <typename accessor, auto res = lookup_field<metadata>(as_sv(accessor::field_id))>
//     requires (res.has_value)
//   constexpr auto operator[](accessor) const -> const view_read_t<meta::type_of<res->id>, order, B>;
//
// array_of_records_view<S, N, order, B>:
//   constexpr auto operator[](std::size_t i) const -> const struct_view<S, order, B>;   // unchecked
//   static constexpr auto size() -> std::size_t;                                       // N
}
```

| Name | Decision and reason |
|---|---|
| **`struct_view_le` / `struct_view_be`** | **Mirror `struct_cast_le` / `struct_cast_be` name for name.** The spec asks for "its own entry point pair mirroring the parser's". The README roadmap already calls the feature `struct_view` (README.md:255), so a reader who knows the roadmap finds it under that name. |
| **`struct_view<S, order, B>`** | **The view type, distinct from the entry-point names.** A function template and a class template cannot share a name in one namespace, so the type is `struct_view` and the calls carry the `_le`/`_be` suffix, like `std::expected` and `std::unexpected`. |
| **The sub-view's exact type** | **Also `struct_view<Nested, order, B>`: the same template, not a separate sub-view class.** A sub-view is a view of the nested schema whose byte pointer is the parent's plus a compile-time offset. One type means a function taking `struct_view<elf_section_header, …>` accepts a top-level view and a sub-view alike. This is what the inbox brief on runs of records will want as its element type. |
| **`array_of_records_view<S, N, order, B>`** | Forced by the syntax `v["entries"_f][i]`: `struct_view::operator[]` takes a field accessor, so something else must take `std::size_t`. The name follows the descriptor (`array_of_records`). `size()` is the one addition beyond `[i]`. The precondition is `i < N`, and without `size()` a caller can only satisfy it by repeating the schema's constant. There are no iterators. |
| **`wire_width<S>`** | A free variable template, not a member of `struct_view`. A caller stepping through records (pcap: `wire_width<hdr> + incl_len`) needs the stride *before* it has a view. Spelling it as a member would make it name an order and a byte type that have nothing to do with width. "Wire" is the spec's word, and it distinguishes the value from `sizeof`. The spelling follows the `bound_in_bytes<…>` precedent: a constant with no `_v`. |
| **Template parameter `order` is `std::endian`** | Not `cast_endianness`. `cast_endianness` is host-relative (`host`/`foreign`), so `struct_view<S, host, char>` would mean little-endian on x86 and big-endian on s390. A type the user may spell in a function signature has to mean the same on every host. The view collapses `order` with the existing `deduce_byte_order<order>()` at the point of use. |
| **Template parameter `B` is the span's element type, const removed** | Forced by constant evaluation. Moving from `const char*` to `const unsigned char*` or `const std::byte*` needs a `reinterpret_cast`, which is illegal in a constant expression. Erasing the byte type would therefore lose constexpr creation for two of the three accepted element types. |
| **`raw_byte_like<B>`** | `char`, `unsigned char` or `std::byte`, **const or not**. The spec says "read-only `std::span`". The view never writes, so it accepts the read-only spelling. It also accepts a mutable one, because template deduction will not convert `std::span<char>` to `std::span<const char>`. Requiring `const` would force the caller in the user story ("a `char` buffer from a file … so that I do not cast") to spell exactly that conversion. `signed char` is excluded: it is not one of the three types that may alias any object. |
| **No public constructor** | `struct_view` and `array_of_records_view` construct only through friends: other `struct_view` and `array_of_records_view` specializations, and `struct_view_impl`. A public constructor taking a pointer would be a second creation path that skips both the bounds check and the constraint check. That would contradict the "checked once, at creation" contract the docs must state. |
| **Primary template declared, not defined, and unconstrained** | One partial specialization on `struct_field_list_impl<metadata, fields...>`, exactly as `struct_cast_impl` does it. The guarantee that `S` is a fixed layout comes from the constrained entry points and the private constructor. It does not come from constraining the class. |

The returns are **`const` values** throughout, as the spec requires: for
scalars, arrays, strings, sub-views and array views alike. **(verified)**
Neither gcc 14 nor clang 19 warns on a dependent `const T` return under
`-Wall -Wextra -Wpedantic`, so the `-Werror` doc-example and shipped-header
targets stay clean. The assignment `v["arr"_f] = {...}` fails as gcc's
"passing 'const std::array<…>' as 'this' argument discards qualifiers" and as
clang's "… but method is not marked const". §7.3 uses those two phrasings as
the match.

`struct_view::operator[]` carries **no visibility policy**, unlike
`struct_field_list_impl::operator[]`. Every field kind that policy exists for
is excluded by the fixed-layout rule:

- **Length targets** exist only to feed `vec_field`, `str_field` and `vector_of_records`, all rejected.
- **Discriminants** exist only to feed `variance`, which is rejected.
- **Frozen fields** come back as `const` values here anyway.

---

## 2. The fixed-layout rule

### 2.1 The concept and its sub-concepts

```cpp
template <typename S> concept has_no_announcing_record           = has_no_announcing_record_v<S>;
template <typename S> concept has_no_optional_field              = has_no_optional_field_v<S>;
template <typename S> concept has_no_variant_field               = has_no_variant_field_v<S>;
template <typename S> concept every_field_has_compile_time_width = every_field_has_compile_time_width_v<S>;

template <typename S>
concept fixed_layout_schema =
  field_list_like<S> &&
  has_no_announcing_record<S> &&
  has_no_optional_field<S> &&
  has_no_variant_field<S> &&
  every_field_has_compile_time_width<S>;
```

| Sub-concept | Rejects | Atom computes |
|---|---|---|
| `has_no_announcing_record` | `announces_byte_order` at any depth | `census_of_list_v<S>.on_spine + .off_spine == 0`. This **reuses** the census, which is already pure and already descends through every record-carrying kind. It is the same predicate `fixed_order_check` computes, without the `static_assert`. |
| `has_no_optional_field` | every `maybe` | no field is `optional_field_like` |
| `has_no_variant_field` | every `variance`, including one whose alternatives share a width | no field is `union_field_like` |
| `every_field_has_compile_time_width` | `vec_field`, `str_field`, `vector_of_records` (counted or sentinel-terminated) | every field is a fixed-width leaf (`fixed_sized_field_like`), or a `struct_field_like` / `array_of_record_field_like` whose nested schema satisfies this same atom |

"Variant", not "union", in the third name. The docs page is "Optional and
variant fields" and the descriptor the user wrote is `variance`. A diagnostic
should use the vocabulary the author typed. The code-level trait
`union_field_like` stays internal.

**Every `_v` atom has a primary of `false`** and a single partial
specialization on `struct_field_list_impl<metadata, fields...>`. Each
sub-concept is therefore pure for *any* argument, not just for field lists.
This matters for `has_no_announcing_record_v`: `census_of_list` has no
primary definition, so calling it directly on a non-list would hard-error.
The `_v` primary is what guards it.

**Descent.** `has_no_announcing_record_v` descends everywhere, for free, via
the census. The other three atoms descend only through `struct_field` and
`array_of_records`, the two record-carrying kinds a fixed layout admits.
Descending into a `vector_of_records`, `maybe` or `variance` could only change
*which* name is reported for a schema the outer field already rejects, never
whether it is rejected. So that extra code buys nothing. 080's nested cases
put the offender under a `struct_field` and inside an `array_of_records`
element, which are exactly the paths that descend.

### 2.2 Conjunct order is the diagnostic

Constraint satisfaction stops at the first false conjunct, and both compilers
report only that one **(verified)**. The width rule is false for all six
rejected kinds:

- a `maybe_field` is not `fixed_sized_field_like`
- an `order_announcing_field` is not `struct_field_like`
- a `union_field` matches neither

So it must come **last**. Otherwise `maybe`, `variance` and
`announces_byte_order` would all be reported as a width problem. Each
specific exclusion comes before it, so each rejected kind fails on exactly
the name that describes it.

### 2.3 Why each sub-concept is a single named atom: gcc's default diagnostic depth

**(verified)** At its default `-fconcepts-diagnostics-depth=1`, gcc 14 does
not print the failing concept's definition. It prints the failing **atomic
expression**, for example: "the expression 'has_no_optional_field_v<S>
[with S = …]' evaluated to 'false'". For a disjunction it prints "no operand
of the disjunction is satisfied" and **no name at all**.

So a sub-concept written as an inline expression, such as `(!optional_field_like<fields> && ...)`,
would reach a gcc user as an anonymous fold. The rule here is that **each
sub-concept's body is exactly one variable-template atom whose name is the
rule**. That holds for `span_covers_record`'s disjunction too: it is wrapped
in the atom `span_covers_record_v`. Clang prints the concept chain *and* the
atom. gcc prints "required for the satisfaction of 'has_no_optional_field<S>'"
and then the atom. Both name the rule.

That rule also fixes the `must_not_compile` matching (§7.3). **(verified)**
Clang echoes the source line of each concept in the chain, so the
composite's definition line, with every sub-concept name on it, appears in the
output whatever failed. A regex on a bare sub-concept name can therefore
false-match. The **atom** name (`has_no_optional_field_v`) appears in the
output if and only if that atom failed. It is not in the candidate-signature
line, which shows only `fixed_layout_schema<S>` and `span_covers_record<…>`.
It is not in the composite's definition line either, which composes
sub-concepts by name and never mentions an atom. Two consequences:

- **The composite must never name an atom directly.**
- **No `must_not_compile` source may mention an atom name in a comment.**

### 2.4 Deliberate divergence from `fixed_order_schema`

`fixed_order_schema` and `self_announcing_schema`
(`announcing_record.hpp:263-295`) are each backed by a struct of
`static_assert`s: `fixed_order_check` and `self_announcing_check`. A concept
reads that struct's `res`. Evaluating the concept on the wrong kind of
schema therefore *instantiates* the struct, and the `static_assert` fires. It
is a hard error, never `false`. That buys a full sentence of explanation
("call struct_cast to read it, or stream_cast to write it"). It costs any
negative test: `static_assert(!fixed_order_schema<announcing>)` does not
compile. That is why those concepts have no negative trait tests, and why
their `must_not_compile` cases match the `static_assert` text.

`fixed_layout_schema` contains no `static_assert`, anywhere in its closure,
on purpose:

1. **The spec requires it.** The concept is tested as a trait both ways
   (`static_assert(!fixed_layout_schema<rejected>)`), which only a pure
   concept permits.
2. **It is a constraint whose `false` must mean "not viable".** A hard error
   inside a constraint ends compilation at the constraint rather than
   reporting a failed candidate.
3. **The view entry point must not use `fixed_order_schema` at all.** On an
   announcing schema it would hard-error *before* `has_no_announcing_record`
   could evaluate to `false`. Its sentence would also tell the user to call
   `struct_cast`, which is wrong advice for someone asking for a view. So
   `has_no_announcing_record` reuses the census predicate (`on_spine == 0`;
   `off_spine` is always 0 in a declarable schema) without the struct around
   it.

What is given up: the explanatory sentence. A view's diagnostic names the
rule; it does not argue for it. 082 compensates. The reference page lists
each sub-concept and its atom by name, with what it rejects and why, so the
name in the error leads to the explanation.

This is a pattern worth recording for future work. A **pure, diagnosable
concept** is a composite of sub-concepts, each one atom named for its rule,
ordered most-specific first. It sits beside the older pattern of a struct of
`static_assert`s plus a `res`, and the next author should choose between them
deliberately. The deciding question: must the concept be testable as `false`?

### 2.5 Growing the rule across 075–079 without an unsafe intermediate state

The issues widen the accepted subset one slice at a time. The concept must
never accept a schema the code on `main` cannot honour. In particular, a
constrained field accepted before 077's constraint walk exists would be
silently unvalidated. So the leaf test in `every_field_has_compile_time_width_v`
goes through one field-level predicate, narrowed per slice:

| After | Leaf admitted | Also admitted | Composite |
|---|---|---|---|
| 075 | `fixed_sized_field_like` **and** `trivial` field type **and** `is_no_constraint` | — | `field_list_like && every_field_has_compile_time_width` |
| 076 | `fixed_sized_field_like` **and** `is_no_constraint` | — | same |
| 077 | `fixed_sized_field_like` (magic fields and any constraint) | — | same |
| 078 | same | `struct_field` over an accepted schema, with or without its own constraint | same |
| 079 | same | `array_of_records` over an accepted schema, with or without its own constraint | same |
| 080 | same | same | all five conjuncts (§2.1) |

Before 080, the width atom alone already rejects `announces_byte_order`,
`maybe` and `variance` (§2.2), so every intermediate state is safe. 080 adds
the three exclusion sub-concepts in front, for the diagnostic and the
negative tests. It does not change what is accepted. The width atom's name is
inaccurate for a constrained leaf during 075–076. That is tolerable because
nothing asserts it negatively before 080, by which point the narrowing is
gone. That is why `is_no_constraint` lands in 075, not 077.

---

## 3. Offsets: a constexpr table per schema

**Decision: one constexpr offset array per schema, over a lowered value.** It
is not offsets recomputed by template recursion per accessed field.

`record_layout<struct_field_list_impl<metadata, fields...>>` holds:

- `widths`: `std::array<std::size_t, sizeof...(fields)>` of each field's wire width:
  - a leaf: `deduce_field_size<F::field_size>{}()`, the existing compile-time size reader, so a narrow `2_B` is 2 and not `sizeof(T)`;
  - a `struct_field`: `record_layout<nested>::width`;
  - an `array_of_records`: `N * record_layout<element>::width`.
- `offsets`: the exclusive prefix sum of `widths`, computed by an ordinary constexpr loop.
- `width`: the sum of `widths`. `wire_width<S>` is this.
- `names`: `std::array<std::string_view, n>{as_sv(fields::field_id)...}`, for `failed_at` (§5).

The accessor finds the field's position through what the parser's
`operator[]` already uses: `lookup_field<metadata>(name)` →
`field_type_info::occurs_at_idx`, which is the field's index in the pack
(`field_list_metadata.hpp:327-334`). It then indexes `offsets` with that
constant. No offset is computed at run time. A nested offset is the parent
pointer plus one constant per level. An element offset is that plus
`i * record_layout<element>::width`, which multiplies a run-time `i` by a
constant, as 079 allows.

**On value lowering.** The brief recorded that this feature benefits most
from value lowering. `record_layout` *is* a value lowering, of exactly one
property: per-field wire width, lowered from the type-level schema into a
`std::array` value that ordinary constexpr code folds over. It needs no
general value IR, so the work is not gated on one, as the spec says. If a
full IR lands, `record_layout` is the single place to re-point.

**Template recursion, rejected:**

- An `offset_of<S, "name">` recursing over the pack would instantiate a fresh O(n) walk per (schema, accessed name).
- It would duplicate the name-to-position lookup `field_table` already performs.
- It leaves nowhere to hang the other two consumers of the same numbers: `wire_width` and the short-span scan (§5.1). Both would need the table anyway.

---

## 4. Reads

### 4.1 One decode path, borrowed from the parser

```cpp
template <typename F, std::endian order, raw_byte_like B>
constexpr auto decode_field_at(const B* at) -> F;
```

It does three things:

1. **Default-constructs `F`.** This is exactly what the parser's
   `struct_field_list_impl` does to each of its bases, so a frozen (`eq`)
   field's seed is in place.
2. **Reads the field's declared byte count into `F::value`** through
   `read_impl(src, f.value, width, deduce_byte_order<order>())`, where
   `src` is an `unchecked_byte_source<B>` positioned at `at`.
3. **Returns `F`.** The `rw_result` is discarded. It cannot fail: creation
   already established that every byte of the record is inside the span. Per
   the global rule, there is no handling for the impossible case.

`unchecked_byte_source<B>` derives from `constexpr_stream`, so `read_impl`
takes `read_native_impl`'s **constexpr arm**: a `bit_cast` of a
`std::array<char, sizeof(T)>`. It takes that arm during constant evaluation
*and* at run time. Its `read(std::array<char, N>&, n)` copies `n` elements
from the pointer (each `static_cast<char>`) and advances. Its `operator bool`
is `true`. Its name states that it does not bounds-check.

Why reuse `read_impl` wholesale instead of writing a ten-line copy plus
`bit_cast` plus `byteswapped`:

- **Equivalence by construction.** Narrow `N_B` placement, foreign scalar
  swap (`read_foreign_scalar`), element swap (`read_foreign_buffer` →
  `byteswap_elements`), the C-array branch, `fixed_string` passing through
  unswapped: all of it is the parser's own code. Two quirks surfaced while
  checking this (§12 F1, F2). Both are reproduced automatically, and a
  hand-written decoder would have diverged on both.
- **One arm in both contexts.** The ut compile-time run exercises exactly
  the code that runs at run time.
- **No `if consteval` needed.** Nothing in `include/` or `test/` uses
  `if consteval` or `std::is_constant_evaluated()`, and this does not
  introduce the first. The run-time arm's `reinterpret_cast<char*>(&obj)` is
  not constexpr, so it could never serve both contexts.

**Alignment.** No typed pointer into the buffer is formed anywhere:

- The source is read element by element as `B`, a byte type that may alias anything.
- The destination is a correctly aligned local.
- A sub-view is a `const B*`, never a typed record pointer.

**(verified)** A byte-copy-and-`bit_cast` load at an odd address is clean
under `-fsanitize=undefined` on gcc 14 and clang 19 at both -O0 and -O2. A
`*reinterpret_cast<const u32*>` at the same address is a fatal report on
both. That is the regression 081 exists to catch.

"Memcpy-like at run time": the run-time copy is a byte loop into a staging
array plus a `bit_cast`, which gcc and clang fold into a plain load at -O2.
The spec has no timing gate (Non-Goals), and the structural property it does
have, a copy into a typed local, holds either way.

### 4.2 What an accessor returns: `view_read_t<F, order, B>`

| Field kind | Returns (as `const`) | How |
|---|---|---|
| `basic_field`, `magic_number` | `T` | `decode_field_at<F>(…).value` |
| `fixed_array_field`, `magic_byte_array` | `std::array<T, N>` | same |
| `fixed_string_field`, `magic_string` | `fixed_string<N>` | same |
| `c_arr_field` | `std::array<T, N>` | `std::to_array(decode_field_at<F>(…).value)`; a function cannot return `T[N]` |
| `c_str_field` | `std::array<char, N + 1>` | same |
| `struct_field` | `struct_view<Nested, order, B>` | pointer + `offsets[idx]`; nothing decoded |
| `array_of_records` | `array_of_records_view<Elem, N, order, B>` | pointer + `offsets[idx]`; nothing decoded |

C-array fields are decoded into their own `T[N]` (through `F`) and then
converted. They are not read straight into `std::array`. One decode path
serves both the accessor and the constraint walk. The walk *must* see `T[N]`,
because a `c_arr_field`'s constraint is a `constraint_option_like<C, T[N]>`.
The conversion costs one copy of a small array.

### 4.3 Laziness

Reading `v["hdr"_f]["len"_f]` performs two pointer additions by constants
and one `decode_field_at` of one leaf. Reading
`v["entries"_f][i]["flags"_f]` performs one more addition, of `i * width`.
No other field of any enclosing or sibling record is touched.

### 4.4 The sub-view's exact type, restated

`struct_view<Nested, order, B>` holds one `const B*`: the first byte of the
nested record. The spec's "byte pointer and an offset" is satisfied with the
offset already folded into the pointer at the point of creation. A
`(base, offset)` pair would be two members expressing what one already does.
`struct_view` is trivially copyable, `sizeof(struct_view) == sizeof(const B*)`,
and it owns nothing. 075 asserts all three with `static_assert`s.

---

## 5. Creation: `struct_view_impl<S, order>`

Creation proceeds in three steps:

1. **Length check.** This applies to a dynamic extent only; a static extent
   was settled by `span_covers_record`.
2. **Constraint walk.** This is skipped entirely, at compile time, when
   `!any_field_constrained_v<S>`. A pcap record header has no constrained
   field, so its creation is the length check and nothing else.
3. **Construct and return** `struct_view{bytes.data()}`.

Creation succeeds or fails as a unit. No partially validated view escapes.

### 5.1 `failed_at` for a short span

**Decision: `failed_at` names the first top-level field that does not fit.**
That is the first `k` with `offsets[k] + widths[k] > bytes.size()`, found by
scanning `record_layout<S>`'s two constexpr arrays. `failure_reason` is
`buffer_exhaustion`. This scan runs on the failure path only.

Why this, over the alternatives:

- **It is exactly the parser's answer** whenever the parser would also have
  reported `buffer_exhaustion`. On running out, the parser names the
  top-level field it was reading; a nested exhaustion is stripped to the
  outermost name by `read_field<struct_field_like>` and by
  `read_buffer_of_records`. For a fixed layout, that field is the first one
  whose end passes the span's end. The view therefore differs from the parser
  only where the spec already said it may: when the parser would first have
  reported `validation_failure` on an earlier field, which the view never
  evaluates because the length check comes first.
- **It keeps a documented invariant.** `docs/errors.md` § "What `failed_at`
  names" says "`failed_at` is always a field id that appears in the schema
  passed to the cast". An empty `failed_at` would break that.
- **Rejected:** `""`, for that reason. Always the *first* top-level field,
  because it is uninformative and wrong against the parser even in the case
  where the parser agrees.

Such a `k` always exists for a short span, because the last field ends at
`width > size`. 081's one-byte-short test therefore names the last field
(in a flat record of non-zero-width fields).

### 5.2 The constraint walk and why it matches the parser

"Constrained" means `!is_no_constraint_v<std::remove_cvref_t<decltype(F::constraint_checker)>>`.
That is the same `remove_cvref_t<decltype(…)>` shape `frozen_field_like`
already uses with `is_eq_constraint_v`. Magic fields (`eq`) are constrained.
A user lambda that always returns `true` is constrained, and is evaluated.

For each top-level field `k`, in wire order, the walk asks: *does this
field, at any depth, violate a constraint?* The first `k` that does produces
`cast_error{validation_failure, names[k]}`. Per field kind:

- **A leaf**, if constrained: `!F::constraint_checker(decode_field_at<F, order>(at).value)`.
  This is the parser's own expression from `struct_cast_impl.hpp:38`, applied
  to a value decoded the parser's way.
- **A `struct_field` without its own constraint**: recurse into the nested
  schema's constrained fields, at the nested offsets.
- **A `struct_field` with its own (record-level) constraint**: decode the
  whole nested record with the parser itself,
  `struct_cast_impl<Nested, unchecked_byte_source<B>>{}(src, order_cell)`.
  The record is violated if that fails (a nested constraint) or if
  `F::constraint_checker(*res)` is false. A record-level constraint is a
  constraint over the whole record, so decoding the whole record is what
  "decode only constrained fields" means for it.
- **An `array_of_records` without its own constraint**: for every element
  `i`, recurse into the element schema at `at + i * width`. This is guarded
  by `if constexpr (any_field_constrained_v<Elem>)`, so an array of
  unconstrained records costs no loop at all, neither at -O0 nor against the
  constant-evaluation step budget. That is 079's "bounded by the number of
  constrained fields, not the number of fields".
- **An `array_of_records` with its own constraint**: decode every element
  with `struct_cast_impl`, assemble the `std::array<Elem, N>`, and evaluate.

**Parity argument.** It is short, and it is what the tests rely on.

1. For a span at least as long as the record, the parser cannot fail with
   anything but `validation_failure`:
   - no read can run short;
   - there is no `variance`, so no `type_deduction_failure`;
   - there is no variable size, so no `excessive_length` and no `found_contradicting_length`.
2. Every failure inside top-level field `k` is reported as `names[k]`.
   `read_field<struct_field_like>` and the record-sequence readers strip the
   inner name (`field_reader.hpp:239-241`), and the outer fold re-attaches
   the top-level one (`struct_cast_impl.hpp:33-35`).
3. So the parser's error is fully determined by *which top-level field first
   contains a violation*. The order of evaluation *within* that field cannot
   change it.
4. The walk reports the same top-level field. Its within-field order matches
   the parser's anyway: nested fields before the record-level constraint,
   elements in index order.

Frozen-field seeding (§12 F2) is reproduced because both sides
default-construct `F` before reading.

### 5.3 Error propagation, layer by layer

| Seam | Representation in | Representation out | What the upper layer may know |
|---|---|---|---|
| byte source → `read_impl` | — | `rw_result`, always success | nothing; discarded at `decode_field_at` (impossible case) |
| `struct_cast_impl` (record-level constrained aggregate) → walk | `std::expected<Nested, cast_error>` | `bool` violated | that the field is violated; not where inside it, as with the parser |
| walk → creation | `bool` per top-level field | `cast_error{validation_failure, names[k]}` | the top-level field only |
| length → creation | `bytes.size()` vs the `record_layout` arrays | `cast_error{buffer_exhaustion, names[k]}` | the first field that does not fit |
| concept / extent → caller | — | candidate not viable; the diagnostic names the atom | which rule (§2.3) |
| accessors → caller | — | a value; there is no error channel | nothing can fail. Lifetime and index are documented preconditions; violating one is UB. |

The representation changes only at the creation boundary, from a `bool` to a
`cast_error`. The walk never builds a `cast_error` itself, which is what
keeps the outermost-name rule in one place.

---

## 6. The UBSan configuration (074)

**Decision: a CMake option, driven by a CMake workflow preset, selecting
tests by a CTest label.** The documented command, run from the repo root:

```sh
CXX=g++-14     cmake --workflow --preset ubsan --fresh
CXX=clang++-19 cmake --workflow --preset ubsan --fresh
```

It is one command, which configures, builds and runs the tests.
**(verified)** CMake 3.28 here accepts `cmake --workflow --preset … --fresh`.
Workflow presets need CMake ≥ 3.25. The root `cmake_minimum_required(3.20)`
does not change; only the one-command form needs 3.25.

Three pieces, each forced:

1. **`option(S2S_SANITIZE_UNDEFINED … OFF)` in `CMakeLists.txt`.** When on,
   it adds `-fsanitize=undefined -fno-sanitize-recover=all` to compile
   options and `-fsanitize=undefined` to link options, before
   `add_subdirectory(test)` so every target is covered. *Why an option and
   not flags in the preset:* `CMakeLists.txt:22` does
   `set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")`. That normal variable
   shadows the cache entry, so flags a preset passes through
   `CMAKE_CXX_FLAGS` would be dropped silently. The result would be a green
   run that sanitized nothing, which is the class of false pass the project
   already guards against (`does NOT link in any test case`,
   `ct_coverage_check.cmake`). `-fsanitize=undefined` includes `alignment`
   on both compilers. With MSVC and the option on, configure stops with
   `FATAL_ERROR` rather than silently running unsanitized. An MSVC configure
   with the option off, which is the default, is untouched.
2. **A `run_time` CTest label** set in `add_struct_cast_test` (every
   GoogleTest entry) and on `add_ut_test`'s `${name}_run_time` entry.
   *Why a label and not `-E '_compile_time$'`:*
   - **The excluded setups come back.** `${name}_coverage` has
     `FIXTURES_REQUIRED ${name}_built`, and CTest automatically re-adds the
     fixture's setup test (`${name}_compile_time`) when a requiring test is
     selected. Excluding `_compile_time` by regex therefore does not exclude
     it.
   - **The regex selects too much.** A regex would also run the
     `must_not_compile` builds and the doc checks, which gain nothing from
     instrumentation.
   - **No regex can name the GoogleTest entries.** They share no name
     suffix. The label selects exactly the spec's set: the GoogleTest suite
     plus the ut `_run_time` entries. `_run_time` deliberately has no
     fixture, so nothing is pulled back in.
3. **`CMakePresets.json`** (schema version 6) with:
   - a configure preset `ubsan`: `binaryDir` `${sourceDir}/build-ubsan`, sets `S2S_SANITIZE_UNDEFINED=ON`;
   - a build preset `ubsan`;
   - a test preset `ubsan`: filter `include.label = ^run_time$`, `outputOnFailure`;
   - a workflow preset `ubsan` chaining the three.

   The compiler comes from `CXX`, as for any configure. This host's default
   `g++` is 13, which the root `CMakeLists.txt` rejects, and compiler
   executable names vary by machine, so the preset does not pin one.
   `--fresh` is part of the documented command because CMake caches the
   compiler on first configure and silently ignores a changed `CXX`
   afterwards.

   The build step should be parallel (`jobs`, or `CMAKE_BUILD_PARALLEL_LEVEL`
   in the environment). The `*_compile_time` targets are in `ALL`, and the
   tree takes a long time to build single-threaded.

**Build type: the project default** (`RelWithDebInfo`, -O2). **(verified)**
The typed misaligned load is caught at -O2 as well as -O0. The `-Werror`
targets (shipped header, doc examples, `hello_s2s`) stay warning-clean under
UBSan at both levels on both compilers. So there is no reason to diverge from
the build developers run.

**(verified) Viability.** The `_compile_time`, `_run_time` builds of
`trivial_read_ct`, `record_read_ct` and `fixed_buffer_read_ct` all build and
pass under `-fsanitize=undefined -fno-sanitize-recover=all` on both
compilers. Instrumentation does not disturb constant evaluation, so global
instrumentation is safe. The full-suite run is 074's job, as is its rule:
list any pre-existing report rather than suppress it.

**Blocked on clang today (§12 F4).** The `-Werror` targets pass the
gcc-only `-Wno-non-template-friend`, which clang rejects as an unknown
warning option under `-Werror`. That happens with or without a sanitizer, so
clang cannot build the tree as it stands. 074's "applies to the gcc and
clang toolchains" cannot be met until those flags become
`$<$<CXX_COMPILER_ID:GNU>:-Wno-non-template-friend>`. **Recommendation: fix
it inside 074.**

**Where it is written down:** README § "Building and running the tests",
where `ctest` usage already lives, plus one line in `AGENTS.md`'s testing
paragraph telling implementers to run it for view slices. 082 checks both.

---

## 7. Tests

### 7.1 Layout

| File | Framework / helper | Holds | Slices |
|---|---|---|---|
| `test/schema/struct_view_read_ct.cpp` | ut, `add_ut_test` | view vs `struct_cast_le`/`_be` over a `memstream` of the same `std::array<u8, N>`, field by field, both orders. Covers every accepted kind, nested, arrays of records, narrow `N_B`, floats, and C arrays elementwise. Also: error parity, the short span, misaligned offsets, mutation of untouched fields. | 075–079, 081 |
| `test/schema/struct_view_read.cpp` | GoogleTest, `add_struct_cast_test` | the same matrix at run time. Bytes are written with `PREPARE_INPUT_FILE` and read back from the file into a `std::vector<char>` (dynamic extent) and a `std::array<char, N>` (static extent). Compared against `struct_cast_le`/`_be` over an `std::ifstream` of that file. Also the odd-address case: `alignas(8)` storage, span starting at `+1`. | 075–079, 081 |
| `test/schema/struct_view_allocation.cpp` | GoogleTest only | replaces global `operator new`/`operator delete` with counting versions, then asserts a zero delta across creation and reads (§12 F3) | 081 |
| `test/internals/fixed_layout_schema_ct.cpp` | ut, `add_ut_test` | `static_assert` positives and negatives for `fixed_layout_schema` and each sub-concept, plus `wire_width` and `record_layout::offsets` values. It lives in `internals/` for the reason `announcement_census_ct.cpp` does: it asserts a type-level computation, not a read. | 075 (positive), 080 (negative), 078/079 (nested widths) |
| `test/must_not_compile/struct_view_misuse.cpp` | `add_rejected_case_matching` | §7.3 | 075, 076, 080 |

There is one pair in `test/schema/`, not a pair per slice, because the spec
names one ("a `<feature>_ct.cpp` ut suite and a GoogleTest counterpart").
`record_read_ct.cpp` is the precedent for one file covering several
record forms. The pair is named `struct_view_read{,_ct}` per the directory's
`<feature>_read` convention. §12 F6 notes that a view is an API rather than a
schema construct, for 082's `AGENTS.md` pass.

The allocation test is GoogleTest-only and in its own file because:

- **Replacing global `operator new` affects the whole binary.** Keeping it out
  of `struct_view_read.cpp` keeps that file's intent single.
- **An allocation count is not constant-evaluable.** Constexpr allocation is
  legal if freed within the evaluation, so a successful constant evaluation
  proves nothing. That is the same constraint-driven split
  `allocation_bound_default_override_ct` documents in the other direction.

The equivalence tests compare `cast_error`s by member (§12 F7). The mutation
tests build two buffers that differ only in an unconstrained, unread field
(top level and at depth) and assert identical creation results and reads.
They must not place that field inside a `struct_field` or `array_of_records`
that carries its own constraint: such a field *is* decoded at creation, by
design (§5.2).

### 7.2 UBSan coverage

Every entry above in `test/schema/` and `test/internals/` is registered
through the two helpers, so it carries the `run_time` label from 074 with no
per-file action. This covers the whole view suite, the spec's "ideally".
081's by-hand check temporarily swaps `decode_field_at`'s source for a typed
load and records in the issue file that `cmake --workflow --preset ubsan`
fails.

### 7.3 `must_not_compile` cases and their regexes

| CASE | What | Regex | Slice |
|---|---|---|---|
| 1 | `std::span<const char, wire_width − 1>` | `span_covers_record_v` | 075 |
| 2 | `v["arr"_f] = std::array<…>{}` | `discards qualifiers\|not marked const` | 076 |
| 3 | `vec_field` | `every_field_has_compile_time_width_v` | 080 |
| 4 | `str_field` | `every_field_has_compile_time_width_v` | 080 |
| 5 | `vector_of_records` | `every_field_has_compile_time_width_v` | 080 |
| 6 | `maybe` | `has_no_optional_field_v` | 080 |
| 7 | `variance` whose alternatives share one width | `has_no_variant_field_v` | 080 |
| 8 | `announces_byte_order` | `has_no_announcing_record_v` | 080 |
| 9 | a `maybe` one `struct_field` deep | `has_no_optional_field_v` | 080 |
| 10 | a `vec_field` inside an `array_of_records` element | `every_field_has_compile_time_width_v` | 080 |

Regexes are on **atom** names, per §2.3; the case source must not mention
them. Every case is checked against a no-`CASE` control build before
registration, as the directory's policy requires.

The comment above `add_rejected_case_matching` says it matches "our own
static_assert text rather than the compiler's concept wording". That should
be widened to "our own identifiers". An atom name is ours. Compilers only
quote it.

CASE 2's regex *is* compiler wording. It is verified for gcc 14 and clang 19
only. MSVC wording for every case is unverified (§12 F10).

---

## 8. Documentation (082)

- **Guide page `docs/views.md`**, in the nav after "Reading". It covers:
  - the not-zero-copy statement in the spec's terms (saves untouched fields only; buys little for a whole-record reader);
  - the accepted and rejected kinds, with each rejection's reason;
  - the `std::array` return for `c_arr_field` and `c_str_field`;
  - `wire_width` stepping;
  - the unsafe contract.

  One complete program, `test/doc_examples/guide_views_example.cpp`, uses a
  realistic fixed-layout format (an ELF64 section header table, or pcap
  record headers), `std::ifstream` opened `std::ios::binary`, and the
  `docs:` / `docs-begin` binding.
- **`docs/reference.md`**:
  - "The entry points" gains the two signatures;
  - a new "Views" section covers `struct_view::operator[]`, `array_of_records_view`, `wire_width` and `fixed_layout_schema`;
  - the four sub-concepts are listed **with their atom names**, so a diagnostic leads to its explanation (§2.4);
  - the unsafe contract is repeated at each entry point and accessor entry, per the spec.
- **`docs/errors.md`** § "What `failed_at` names" gains the short-span rule from §5.1.
- **README.md**: a Features bullet. The roadmap line "struct_view - Zero copy
  views into buffers" is ticked and reworded, since "zero copy" is precisely
  what the spec says this is not (§12 F9). The sanitizer command goes in
  "Building and running the tests".
- **AGENTS.md**:
  - the Context paragraph names the third read API;
  - the testing paragraph gets the sanitizer command;
  - the `test/schema/` row's wording is widened (§12 F6).
- **Contract addition, recommended (§12 F5).** State that constraints, like
  bounds, are checked once at creation. Bytes changed after creation are
  read as they now are, unvalidated.

---

## 9. The required categories

### 9.1 State and data lifecycle

- **Creation is the one unit of success or failure.** The length check and
  every constrained field must all pass, or no view exists. There is no
  partially validated view, and no state survives a failed creation.
- **A read is independent of every other read.** It decodes into a local
  and returns it. Nothing is cached.
- **The view holds a reference to the caller's bytes, not a snapshot.** If
  the bytes change after creation, later reads see the new bytes and no
  constraint is re-evaluated (§12 F5).
- **Reversibility and replay.** Creation is a pure function of
  (bytes, schema, order), so it is replayable by construction: creating
  again over the same bytes gives the same answer. The mutation tests rely
  on exactly that. Nothing persists and nothing is audited, so reversibility
  and audit do not arise.

### 9.2 Error propagation

See §5.3. Errors change representation in exactly one place, the creation
boundary. Each lower layer knows less than the one above it, on purpose:

- the walk knows "violated", never "where inside";
- the decode path knows nothing, because it cannot fail.

Compile-time misuse never reaches run time: an unviewable schema or a short
static span is a non-viable candidate, and the diagnostic names the atom.
Accessors have no error channel. Their preconditions (bytes alive, `i < N`)
are documented, and violating them is UB. During constant evaluation, an
out-of-range index that leaves the span's array is a compile error rather
than garbage. That is worth one sentence in 082.

### 9.3 Concurrency and ownership

**Nothing runs concurrently inside the library, and there is no shared
mutable state.**

- A `struct_view` is an immutable value holding one `const B*`. Its
  accessors are `const` and reentrant.
- `unchecked_byte_source` is a per-read local.
- Every table (`record_layout`, the field tables) is `constexpr`, so it is
  constant-initialized and immutable.
- The caller owns the bytes and their lifetime. Any number of threads may
  read through any number of views over the same bytes concurrently,
  provided no thread writes those bytes meanwhile. That condition is the
  caller's, exactly as with the lifetime condition, and 082 states it beside
  lifetime.

### 9.4 Reuse

Used rather than reimplemented:

| Need | Existing piece | Where |
|---|---|---|
| leaf decode: placement, swap, C arrays, `fixed_string` | `read_impl` → `read_native_impl` / `read_foreign_scalar` / `read_foreign_buffer` → `byteswapped`, `byteswap_elements` | `field_read/read_impl.hpp`, `stream/byte_order.hpp` |
| leaf initialisation, including the frozen seed | `field`'s constructor | `field/field.hpp` |
| leaf width | `deduce_field_size` (compile-time) | `field_size/comptime_field_size_deduce.hpp` |
| field-kind classification | `fixed_sized_field_like`, `struct_field_like`, `array_of_record_field_like`, `optional_field_like`, `union_field_like` | `field/field_traits.hpp` |
| announcing record anywhere | `census_of_list_v` | `field_list/announcing_record.hpp` |
| name → position, name → type | `lookup_field<metadata>`, `field_type_info::occurs_at_idx`, `meta::type_of`; the `operator[]` default-template-argument pattern | `field_list/field_list_metadata.hpp`, `field_list.hpp` |
| constraint evaluation | `F::constraint_checker(value)` | `field_validation/field_value_constraints.hpp` |
| "is this constraint X" trait shape | `is_eq_constraint` | `field_validation/field_value_constraints.hpp` |
| whole-record decode for a record-level constraint | `struct_cast_impl` | `cast/struct_cast_impl.hpp` |
| byte-order collapse | `deduce_byte_order<std::endian>()` | `stream/byte_order.hpp` |
| errors | `cast_error`, `error_reason`, unchanged | `error/cast_error.hpp` |
| nested type extraction | `extract_type_from_field_v`, `extract_type_from_array_v`, `extract_size_from_array_v` | `field/field_metafunctions.hpp`, `lib/s2s_traits/type_traits.hpp` |
| test registration | `add_ut_test`, `add_struct_cast_test`, `add_rejected_case_matching`, `memstream`, `PREPARE_INPUT_FILE` | `test/` |

Introduced here and worth reusing next time:

- **`record_layout<S>` and `wire_width<S>`.** Any positional feature will
  need them: the inbox brief on runs of records (its stride and element
  type), bitfields, seeking.
- **`unchecked_byte_source<B>`.** Any further internal read over memory whose
  bounds were established elsewhere.
- **`is_no_constraint_v`.**
- **The pure-diagnosable-concept pattern of §2.4,** recorded so the next
  constraint author chooses between it and the `static_assert`-struct pattern
  deliberately.

### 9.5 Extension points

- **The contract the inbox brief on runs of records will plug into** is
  `struct_view<S, order, B>` as an element type and `wire_width<S>` as a
  stride. That is why the sub-view is the same type as the view (§1).
- **What counts as an admissible leaf is one predicate**, inside
  `every_field_has_compile_time_width_v`, plus one arm of `view_read_t`. The
  bitfields brief, if it admits a bitfield container, extends those two
  places and `decode_field_at`. Nothing else changes.
- **Byte order is a template parameter, deliberately.** A run-time-order view
  (deferred by the byte-order spec) would be a different type, not a mode of
  this one. Nothing here is shaped to accommodate it.
- **No user-facing customization points.** There are no traits to
  specialize and no pluggable leaf kinds.
- **Stable invariants future work may rely on:**
  - `failed_at` is always a top-level field id of `S`;
  - offsets are compile-time constants;
  - the `const`-value return contract.

### 9.6 Build vs. buy

| Component | Evaluated | Why not |
|---|---|---|
| The view itself | **Boost.Endian** buffer types (`little_uint32_buf_t`, …): unaligned, endian-aware storage | Users would have to redeclare every record as a struct of buffer types, so the s2s schema the spec says to reuse could not be. It also adds a dependency to a dependency-free header-only library, and swaps at the type level rather than per call. |
| | **`std::start_lifetime_as`** (C++23, P2590) | Creates objects in place, but still requires suitable alignment and host byte order, is not usable in constant evaluation, and is not shipped by libstdc++ 14. |
| | **Kaitai Struct** (lazy `instances`); **FlatBuffers** / **Cap'n Proto** (zero-copy accessors) | Each has its own schema language and code generator. FlatBuffers and Cap'n Proto dictate their own wire formats and cannot describe pcap, ELF or MFT. None can consume an s2s schema. |
| Input type | `std::span` (std) | Bought. |
| Decode | `std::bit_cast`, `std::byteswap`, `std::to_array` (std), via the existing `read_impl` | Bought / reused. |
| Sanitizer | gcc/clang `-fsanitize=undefined` | Bought. Nothing third-party. |
| One-command runner | CMake workflow presets (built into CMake ≥ 3.25) | Bought. A shell script was rejected: it would be a second build-entry convention beside CMake's own. |
| Allocation counting | Hand-rolled replacement `operator new` / `operator delete` in one test TU, the standard technique | Bought as a technique. heaptrack/valgrind would add tooling a CTest entry cannot assume. |

### 9.7 Abstractions introduced, each with its forcing problem

| Abstraction | Forced by |
|---|---|
| `fixed_layout_schema` + four sub-concepts | the spec (a named concept of named sub-concepts) and the need to name *which* rule failed |
| one `_v` atom per sub-concept | gcc's default diagnostic depth prints atoms, not concepts; and clang echoes the composite line, making bare names unsafe to match (§2.3, verified) |
| `span_covers_record` + `span_covers_record_v` | the static-extent compile error; "new constraints get a named concept"; gcc prints nothing for a bare disjunction (verified) |
| `raw_byte_like` | the spec's three element types, plus const-or-not (§1) |
| `record_layout<S>` | offsets, `wire_width` and the short-span scan need the same numbers; computed once per schema (§3) |
| `wire_width<S>` | the spec's public width constant |
| `struct_view<S, order, B>` | the spec's view. `B` is forced by constexpr; `order` as `std::endian` by host-independence. |
| `array_of_records_view` | the `[i]` syntax cannot live on a type whose `operator[]` takes a field accessor |
| `view_read_t` | three return shapes (value / sub-view / array view) plus the C-array mapping, in one place the docs can name |
| `decode_field_at` | one decode serving both the accessor and the constraint walk, so the two cannot drift |
| `unchecked_byte_source<B>` | `read_impl` and `struct_cast_impl` take a stream; this is the minimum that adapts a pointer to one |
| `struct_view_impl<S, order>` | creation needs the field pack (partial specialization) and private-constructor access; shared by `_le` and `_be`; mirrors `struct_cast_impl` |
| `is_no_constraint_v` | "evaluate only constrained fields", and 075's interim exclusion (§2.5) |
| `any_field_constrained_v<S>` | 079's cost bound: skip element loops over unconstrained records at compile time |
| `S2S_SANITIZE_UNDEFINED` | `CMakeLists.txt` shadows `CMAKE_CXX_FLAGS`, so no preset or command line can inject flags (§6) |
| `CMakePresets.json` workflow preset | "one documented command" that configures, builds and tests |
| `run_time` CTest label | GoogleTest entries share no name suffix, and a regex re-adds fixture setups (§6) |

Considered and collapsed back to nothing:

- a separate sub-view class;
- a `(base, offset)` pair;
- a run-time offset table;
- iterators on `array_of_records_view`;
- a `bytes()` accessor (§12 F11);
- a visibility policy on `operator[]`;
- a public constructor;
- constraining the class template (§1);
- `operator==` on `cast_error`.

### 9.8 Alternatives rejected

1. **Offsets by per-field template recursion.** See §3.
2. **A distinct sub-view type.** It would give two types for "a view of S"
   and duplicate the accessor. See §1.
3. **A `(base pointer, offset)` sub-view.** Two members for what one
   advanced pointer says.
4. **Erasing the byte type** (always `const unsigned char*`). Converting
   between byte types needs `reinterpret_cast`, which loses constexpr
   creation for `char` and `std::byte` spans.
5. **`cast_endianness` as the order parameter.** The type's meaning would
   depend on the host.
6. **A hand-written decoder** (copy, `bit_cast`, `byteswapped`). It would
   re-derive the parser's narrow placement and seeding and diverge on both
   quirks found (§12 F1, F2). Reuse also makes a future parser fix apply to
   views automatically.
7. **Two byte sources plus `if consteval`, to use `read_native_impl`'s
   run-time arm too.** It adds the codebase's first `if consteval`. The
   run-time arm cannot serve constant evaluation, and one arm in both
   contexts makes the ut compile-time run test exactly the run-time code.
8. **Using `fixed_order_schema` on the view entry, or reusing
   `fixed_order_check`.** It hard-errors instead of evaluating `false`, and
   its message is wrong advice for a view. See §2.4.
9. **A `static_assert`-backed concept like `fixed_order_check`.** It cannot
   be tested as `false`, which the spec requires.
10. **A single whitelist concept with no exclusion sub-concepts.** `maybe`,
    `variance` and `announces_byte_order` would all be reported as a width
    problem.
11. **`maybe` and `variance` sharing one "no optional or union field"
    sub-concept.** It costs one concept less but makes for a less precise
    diagnostic. Separate concepts it is.
12. **A `static_assert` in the entry-point body for a short static extent.**
    It would turn a non-viable candidate into a hard error, against the
    project's "concepts, not SFINAE, and named concepts for new constraints"
    rule.
13. **`failed_at = ""` for a short span.** It breaks the
    `docs/errors.md` invariant and loses parity where the parser agrees.
    See §5.1.
14. **`failed_at` = always the first field.** It is uninformative.
15. **Validating lazily, per read.** The spec settled on eager validation of
    constrained fields only.
16. **Accepting only `const` span elements.** It forces the cast the user
    story exists to avoid. See §1.
17. **Accepting `std::array` / `std::vector` directly.** The spec says the
    input is a span. Overloads for containers would multiply the entry
    points.
18. **UBSan on by default.** 074 requires the default build to be unchanged.
    It would also slow every build.
19. **UBSan flags through a preset alone.** They would be dropped silently;
    see §6.
20. **A CMake option alone, documented as three commands.** That is not one
    command.
21. **Selecting tests with `-E '_compile_time$'`.** CTest re-adds the fixture
    setups, and the regex over-selects. See §6.
22. **Instrumenting only the view targets.** It is more CMake for less
    coverage. Global instrumentation is verified harmless to constant
    evaluation.
23. **Pinning the compiler in the preset.** Executable names are
    machine-specific, and this host's default `g++` fails configure.
24. **A `Debug` build type in the preset.** It is unnecessary: detection and
    `-Werror` cleanliness were verified at -O2.
25. **A checked `at(i)` returning `expected`.** The spec makes indexing
    unchecked. It could be added later without breaking anything.
26. **Returning references into the buffer, or a typed pointer fast path for
    aligned host-order fields.** The first is a spec Non-Goal. The second
    would reintroduce exactly the access 081 exists to forbid.

---

## 10. Decision → issue map

| Decision | Section | Issues |
|---|---|---|
| Entry names `struct_view_le` / `_be`; type `struct_view`; private construction | §1 | 075 (introduce), 082 (document) |
| Sub-view = `struct_view<Nested, …>`; one `const B*`; trivially copyable | §1, §4.4 | 078, 079, 082 |
| `array_of_records_view` with unchecked `[i]` and `size()` | §1 | 079, 082 |
| `wire_width<S>` free constant | §1, §3 | 075 (flat), 078/079 (nested sums), 082 |
| `std::endian` order parameter; `B` element parameter; `raw_byte_like` accepting const and non-const | §1 | 075 |
| `const` value returns; assignment case regex | §1, §7.3 | 075 (scalars), 076 (classes + must_not_compile CASE 2) |
| `fixed_layout_schema` composition, order, `_v` atoms | §2.1–2.3 | 075 (two conjuncts), 080 (five + negatives + CASEs 3–10) |
| Divergence from `fixed_order_schema` | §2.4 | 080 (record in code comment and design), 082 (reference explains each atom) |
| Interim leaf narrowing | §2.5 | 075, 076, 077 |
| `record_layout` constexpr offset table; value-lowering note | §3 | 075, 078, 079 |
| `decode_field_at` over `read_impl` via `unchecked_byte_source` | §4.1 | 075; exercised by 076 (arrays, strings, swap) and 081 (alignment) |
| `view_read_t` C-array mapping via `std::to_array` | §4.2 | 076 |
| `failed_at` = first top-level field that does not fit | §5.1 | 075 (implement), 077 (parity statement), 081 (test), 082 (`errors.md`) |
| Constraint walk; parity argument; record-level constraints via `struct_cast_impl` | §5.2 | 077 (flat), 078 (nested + record-level on `struct_field`), 079 (elements + array-level) |
| `is_no_constraint_v` | §5.2 | 075 (interim exclusion), 077 (walk) |
| `any_field_constrained_v` skip | §5.2 | 077, 079 |
| UBSan: option + workflow preset + `run_time` label; `--fresh`; default build type | §6 | 074; consumed by 075–081 |
| GNU-only `-Wno-non-template-friend` | §6, §12 F4 | 074 |
| Test file layout | §7.1 | 075–081 |
| Allocation counting by replacement `operator new` | §7.1, §12 F3 | 081 |
| `fixed_string::to_sv` const | §12 F8 | 076 |
| Docs placement; contract incl. post-creation mutation | §8, §12 F5 | 082 |

---

## 11. Slice order and dependencies

The order **074 → 075 → 076 → 077 → 078 → 079 → 080 → 081 → 082** stands.
These notes are what the numbering hides:

- **Before 075, a decision is needed on §12 F1** (narrow foreign-order
  placement). If the parser's placement is a bug to fix, fix it first, as a
  `change` that adds a narrow big-endian parser test. Then 075's equivalence
  tests pin the correct value. If it is intended, 075's tests pin it as
  is. Either way the view needs no code change, because it reuses
  `read_impl`.
- **074** must include the clang flag fix (§12 F4), or its clang half cannot
  run. It is otherwise independent and lands first, so 075 onward are
  sanitized from their first commit. The `run_time` label means later slices
  join the sanitized run with no per-slice action.
- **075 brings forward two things the issues place later**:
  - `is_no_constraint_v`, needed for the interim exclusion of constrained leaves (§2.5);
  - the short-static-extent `must_not_compile` case (CASE 1), since 075's AC already requires that compile error.

  With CASE 1 in 075, 081's "if not already covered by 075" bullet is
  discharged.
- **076** adds the `fixed_string::to_sv` const fix (§12 F8), and CASE 2.
- **077** turns on the walk and lifts the interim exclusion in the same commit.
- **078 and 079** must each cover a constraint on the aggregate itself:
  - 078: a `struct_field` carrying a record-level constraint;
  - 079: an `array_of_records` carrying an array-level constraint.

  The 077 AC ("a field carrying any constraint the parser supports")
  implies both, and §5.2 is the mechanism, but neither issue's AC says so
  explicitly.
- **080** depends on 079 only for CASE 10 (the offender inside an
  `array_of_records` element). Its exclusion sub-concepts are independent of
  076–079 and could land any time after 075. Keeping it after 079 lets the
  negative trait tests run against the final accepted set.
- **081** depends on 078 and 079 (misaligned offsets *at depth* and in
  elements) and on 074. It does not depend on 080.
- **082** goes last. It also performs the README and `AGENTS.md` updates
  listed in §8.

---

## 12. Findings: where the spec or the issues are wrong, underspecified, or blocked

**F1 — The equivalence criterion will lock in what looks like a parser bug.
User decision needed.** **(verified)** `struct_cast_be` over
`basic_field<"v", u32, 3_B>` and wire bytes `{01 02 03}` yields `0x01020300`,
not `0x010203`. `read_native_impl` places the `N` bytes at the object's low
addresses, and then `read_foreign_scalar` byte-swaps the *whole* `sizeof(T)`.
The value comes out shifted by `sizeof(T) − N` bytes. No existing test covers
a narrow field in foreign order: every narrow test in `size_axis_read*` is
little-endian.

The spec requires a view to "reproduce the parser's placement of the bytes
within the wider type exactly" and lists "narrow `N_B` … foreign byte order"
in the equivalence criterion. So 075's tests will pin this value for both
APIs. The design reproduces the parser whichever way this is decided (§4.1).
The question is only which value the tests should pin.

**F2 — The narrow frozen-field seed (observation, reproduced).**
**(verified)** `magic_number<"m", u32, 2_B, 0x12345678>` validates over wire
bytes `{78 56}`. `field`'s constructor seeds `value` with the `eq` value, the
read overwrites only the two low bytes, and the seed's high bytes survive
into the comparison. The view reproduces this by default-constructing `F`
before reading (§4.1). This is recorded so nobody "fixes" the view's decode
to value-initialise and breaks parity.

**F3 — 081 cites an approach that does not exist.** 081 says allocation is
"checked with the allocation-counting approach the project already uses for
allocation bounds". The allocation-bound tests (`allocation_bound_read*.cpp`)
assert error codes, not allocation counts. No counting harness exists in the
tree. §7.1 picks the equivalent 081 permits: replacement global
`operator new` / `operator delete` in a dedicated GoogleTest TU. 081's issue
file should state this.

**F4 — Clang cannot build the tree today, which blocks 074's clang half.**
**(verified)** `test/doc_examples/CMakeLists.txt` (every target),
`test/shipped_header/CMakeLists.txt` and `examples/CMakeLists.txt` pass
`-Werror -Wno-non-template-friend`. Clang 19 rejects the unknown warning
option *as an error under `-Werror`*, with or without a sanitizer. 074's
"applies to the gcc and clang toolchains" cannot be demonstrated until those
flags are GNU-only (`$<$<CXX_COMPILER_ID:GNU>:…>`). Clang compiles those
sources warning-free without the flag (verified). This contradicts
`AGENTS.md`'s claim of clang 19 support. Recommend fixing it inside 074, or
as a `change` immediately before it.

**F5 — The unsafe contract omits post-creation mutation.** The contract
lists lifetime, bounds and unchecked indexing. A fourth consequence of
"checked once, at creation" is unstated: constraints are evaluated against
the bytes as they were at creation. A caller who changes the bytes later
reads unvalidated values with no error. Recommend 082 states it beside the
bounds clause. The design treats it as documentation only; no mechanism
changes.

**F6 — `test/schema/` wording.** `AGENTS.md` describes `test/schema/` as
"one schema construct per `<feature>_{read,write}.cpp` … mirrors the
descriptor table". A view is an API, not a construct. The spec places its
tests there explicitly, so the design follows the spec. 082's `AGENTS.md`
check should widen that row's wording.

**F7 — "Equal `cast_error`s" has no `operator==` to call.** `cast_error`
defines none, and the spec's Non-Goals forbid "any change to `cast_error`".
Tests compare `failure_reason` and `failed_at` member by member, as every
existing test does.

**F8 — `fixed_string::to_sv` cannot be called on what a view returns.** It
is declared `constexpr auto to_sv() -> std::string_view const`: a `const`
return type on a *non-const* member function. A view returns
`const fixed_string<N>`, so `v["magic"_f].to_sv()` is ill-formed. The same
already holds for a `const` parsed struct, so this is a latent wart. It
becomes visible the moment 082's example prints a fixed string. Recommend
making it a `const` member in 076. It is a one-line change to an existing
`lib/` header.

**F9 — The README roadmap contradicts the spec.** README.md:255 reads
"struct_view - Zero copy views into buffers". The spec requires the docs to
state plainly that a view is *not* zero-copy. 082 must reword the line, not
just tick it.

**F10 — MSVC diagnostic matching is unverified.** The atom-name regexes were
verified on gcc 14 and clang 19 only. No MSVC is available here. The
directory's own policy comment calls message matching non-portable, and the
spec nonetheless mandates matching on sub-concept names. On an MSVC build
these cases may need `add_rejected_case` (`WILL_FAIL`) instead. The tree's
gcc-only flags (F4) suggest MSVC is not built routinely anyway.

**F11 — 078's prose implies a bytes accessor no AC asks for.** 078 says "a
caller who does want the whole nested record can `struct_cast` over the
sub-view's bytes". `struct_cast` takes a stream, not a span, and this design
exposes neither a `bytes()` accessor on `struct_view` nor a public
span-backed stream. `unchecked_byte_source` is deliberately unchecked and
internal. Treat the sentence as non-binding. A bounds-checked public span
stream is a separate feature, and a reasonable companion for the inbox brief
on runs of records.
