# Design: schema DSL verbosity — value-based sizes and wrapper elision

Spec: `dev/specs/schema-api-verbosity.md` (frozen).
Slices: `dev/issues/043-hide-length-derived-fields-from-operator-index.md`,
`dev/issues/044-value-based-size-axis-behind-existing-spelling.md`,
`dev/issues/045-new-surface-and-atomic-migration.md`.

Everything marked *verified* below was compiled as a standalone prototype with
`g++-14 -std=c++23`, `g++-13 -std=c++23` and `clang++-19 -std=c++23`, all
assertions as `static_assert` so constant evaluation is what was checked, not
just parsing. MSVC was not available on this machine; see Risks.

---

## 0. The shape in one page

A size stops being a type and becomes a **value of an empty (or nearly empty)
structural type**:

| Kind | Value | Type of the value |
|---|---|---|
| fixed width | `2_B`, `byte_count{n}` | `byte_count` |
| length read from a field | `len_from_field<"len">` | `field_accessor<"len">` |
| size computed from fields | `size_from_fields<f, "row", "col">` | `size_from_fields_t<f, fixed_string_list<"row","col">>` |
| irrelevant (records, unions) | `size_dont_care` | `size_dont_care_t` |
| per-alternative widths | `size_choices<a, b>` (internal) | `size_choices_t<a, b>` |

`field`'s third parameter becomes `auto size`; `constraint_on_value` already
is an `auto` NTTP, so the trailing pack is homogeneous and can be reordered
and defaulted. Every *classification* trait keys on the size value's **type**
(`decltype(size)`), never on the value. `deduce_field_size` is the only thing
that keys on the value, because `byte_count` is the only size kind whose
payload is a runtime-shaped member rather than a template parameter.

`field_size<...>`, `fixed<N>`, `with_fields<...>` survive 044 as *normalizing
variable templates* that evaluate to those values, which is what lets 044 land
without retyping a single schema outside `include/`. 045 deletes them.

---

## 1. The constraint everything else is derived from

**A class-type NTTP is a non-deduced context in a partial specialization.**
This cannot be written:

```cpp
template <fixed_string id, typename T, fixed_string len_source, auto constraint>
struct len_obligation<field<id, T, field_accessor<len_source>{}, constraint>>;   // len_source not deducible
```

The argument `field_accessor<len_source>{}` is an expression, not a
template-parameter name, so nothing in it is deduced. This is the single fact
that decides the representation. The workable form deduces the *value* as an
opaque `auto` (which **is** a deduced context when it appears as a bare
parameter name in a class template argument list) and then interrogates its
type:

```cpp
template <fixed_string id, typename T, auto size, auto constraint>
  requires (is_variable_size_v<decltype(size)> && !is_computed_size_v<decltype(size)>)
struct len_obligation<field<id, T, size, constraint>> {
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source_of<decltype(size)>::value);
};
```

*Verified*, including two partial specializations of the same trait that
differ only in the requires-clause on `decltype(size)` (`is_fixed_sized_field`
vs `is_variable_sized_field`), and including recovery of the source id out of
`field_accessor<id>` by a one-line type trait.

**The rule this yields, and the one an implementer must not break: every
parameter a trait needs to pattern-match on must live in the size value's
*type*, not in its value.** Applied:

- `len_from_field` keeps the id in the type (it is `field_accessor<id>{}`, the
  same value `"len"_f` already produces).
- `size_from_fields` keeps the callable and the field-name list in the type
  (`size_from_fields_t<callable, fixed_string_list<...>>{}`), answering the
  spec's first open question: **yes, its value form is a usable structural
  NTTP.** An empty class with only static members and no base classes is both
  literal and structural; `s2s` already relies on this for `no_constraint<T>{}`
  and `eq{...}`. *Verified* as an NTTP argument and in constant evaluation.
- `byte_count` is the deliberate exception: `struct byte_count { std::size_t
  count{}; };` puts the width in the value. No trait pattern-matches a *width*
  — only `deduce_field_size` reads it, and that reads it from the value. Also
  structural (public, non-mutable, structural member). *Verified.*

The exception is what buys the plain literal operator:

```cpp
namespace s2s_literals {
constexpr auto operator""_B(unsigned long long n) -> byte_count {
  return byte_count{static_cast<std::size_t>(n)};
}
}
```

No literal-operator template, no digit parsing, and `basic_field<"v", u16>`
can default its size to `byte_count{sizeof(T)}` with the same expression the
user would have written. Bytes only, per the spec; `bit_count`/`_b` is the
shape a later bitfield feature would take, and nothing here forecloses it.

---

## 2. Trait re-derivation (044)

`field_size.hpp` after the rewrite:

```cpp
template <typename S> struct is_fixed_size { static constexpr bool res = false; };
template <> struct is_fixed_size<byte_count> { static constexpr bool res = true; };

template <typename S> struct is_variable_size { static constexpr bool res = false; };
template <fixed_string id> struct is_variable_size<field_accessor<id>> { ... true };
template <auto f, field_name_list ids> struct is_variable_size<size_from_fields_t<f, ids>> { ... true };

template <typename S> struct is_computed_size { ... };          // size_from_fields_t only
template <typename S> struct is_size_dont_care { ... };         // size_dont_care_t only
template <typename S> struct is_selectable_size { ... };        // size_choices_t<...> only
```

**Traits stay keyed on a type — the type of the size value, not the old
`field_size<fixed<N>>` wrapper.** That satisfies the spec's "none still
pattern-matches `field_size<fixed<N>>` as a type" while keeping the concepts
usable in the two places that need them:

1. as requires-clauses over `decltype(size)` in partial specializations
   (section 1);
2. as *per-element type-constraints on an `auto...` pack*, which section 5
   shows is the difference between a good diagnostic and a bad one. A
   value-parameterised concept (`template <auto v> concept ...`) cannot be
   used as a placeholder constraint at all, so a value-keyed vocabulary would
   have forced a second parallel type-keyed one. One vocabulary, applied
   through `decltype`.

Call sites therefore read `is_computed_size_v<decltype(T::field_size)>`
instead of `is_computed_size_v<typename T::field_size>`. `decltype` of an
`auto` NTTP parameter is not const-qualified, so no `remove_cv` is needed
there — but `decltype` of one of the *normalizing variable templates* of
section 4 **is** `const`-qualified (`const byte_count`), so any trait applied
directly to `decltype(field_size<fixed<4>>)` must strip cv. Keep the stripping
in one place (`size_type_of<...> = std::remove_cvref_t<decltype(...)>`) rather
than sprinkling it; this bit me in prototyping and will bite the implementer.

`deduce_field_size` is the one value-keyed template. It becomes:

```cpp
template <auto size> struct deduce_field_size;                       // comptime_field_size_deduce.hpp

template <auto size> requires fixed_size_like<decltype(size)>
struct deduce_field_size<size> { constexpr auto operator()() const -> std::size_t { return size.count; } };

template <auto size> requires variable_size_like<decltype(size)>     // field_size_deduce.hpp
struct deduce_field_size<size> { constexpr auto operator()(const field_list_like auto&) const -> std::size_t; };
```

Two constrained partial specializations whose argument lists are identical to
the primary's, distinguished only by their constraints. This is legal since
C++20 and *verified on gcc 13, gcc 14 and clang 19* — it is the one construct
in this design I would not have taken on trust. It preserves the existing
split between the two headers (the comptime one must not pull in
`field_list.hpp`) exactly as today.

### The full blast radius

The spec names `field_size.hpp`, the two `*_deduce` headers, and
`derived_value.hpp`. Reading the tree turns up four more files that
pattern-match the size axis and must move in the same slice:

| File | What changes |
|---|---|
| `field/field.hpp` | `field`'s `typename size_type` → `auto size`; member `using field_size = size_type` → `static constexpr auto field_size = size`; `to_optional_field`, `to_field_choice`, `to_field_choices`, `union_field` follow |
| `field/field_traits.hpp` | `is_fixed_sized_field` / `is_variable_sized_field` specialisations constrained on `fixed_size_like size` → `auto size` + requires (*verified*) |
| `field/field_metafunctions.hpp` | `extract_type_from_field<field<id, T, field_size<size>, c>>` → `auto size` |
| `type_deduction/utils/type_tags.hpp` | `as_trivial<T, S>`, `as_vec<T, S>`, `as_string<S>`, `as_vec_of_records<T, S>` take `auto` sizes; `as_struct`/`as_fixed_arr`/`as_fixed_string`/`as_arr_of_records` produce values |
| `type_deduction/utils/helper.hpp` | `size_choices_from_type_conditions` builds the `size_choices_t<vals...>` **type** from per-case size *values* |
| `field_read/field_reader.hpp`, `field_write/field_writer.hpp` | `using field_size = typename T::field_size` → `constexpr auto` + `decltype` at the trait calls |
| `field_list/field_list_metadata.hpp` | all four `extract_*` specialisations keyed on the size shape |
| `api/field_descriptors.hpp` | alias parameters (see 044 shim, then 045 surface) |

`to_field_choices` pairs `std::variant<types...>` with `size_choices_t<vals...>`
— deducing an `auto...` pack out of a *type* is a deduced context and works;
*verified* once the cv-stripping above is applied to the variable-template
spelling.

---

## 3. Slice 043 — hiding length-derived targets

### Two lists, not a tagged list

`field_list_metadata` gains a second `dep_vec`:

```cpp
static constexpr dep_vec length_derived_field_ids     = ...;   // extract_unconditional_len_sources
static constexpr dep_vec discriminant_derived_field_ids = ...; // extract_switch_discriminants
```

with three queries beside them:

```cpp
template <auto md> constexpr auto is_length_derived_field(sv) -> bool;
template <auto md> constexpr auto is_discriminant_derived_field(sv) -> bool;
template <auto md> constexpr auto is_derived_field(sv) -> bool;   // either — unchanged meaning
```

Reasons, since the spec left this open:

- The two extractors already produce two separate `dep_vec`s and
  `generate_derived_field_ids` only exists to concatenate them. Splitting is a
  *deletion* of the concatenation, not new machinery.
- A field can legitimately be both a length target and a discriminant. Two
  lists represent that by membership in both; a single tagged list would need
  either duplicate entries or a bitmask, and `remove_duplicates` (which goes
  through `static_set<sv, N>`) would have to grow an ordering over the tagged
  entry type. That is real code bought for nothing.
- `is_derived_field` — the write path's "this field is overwritten, do not let
  it be assigned" question — stays exactly as it is, as the OR of two lookups.
  The comment in `derived_value.hpp` about there being one source of truth for
  "derived" stays true: the write path keeps asking the general question, and
  only `operator[]` asks the two specific ones.

Cost: one more `static_vector<sv, 64>` per schema, compile-time only.

### `operator[]` after the split

```
mutable  operator[](accessor)         requires present && !is_derived_field           // unchanged
non-const const& operator[](accessor) requires present && is_discriminant_derived     // was: any derived
const    operator[](accessor) const   requires present && !is_length_derived          // was: unconstrained
```

The third line is the one that makes the acceptance criterion true: 043 asks
for *no* overload for a length target, which means the const-object overload
has to exclude it as well, or `const auto& v = fl["len"_f]` would still work.

### The finding that makes or breaks this slice

**Three call sites inside the library read fields through `operator[]`, and two
of them can land on a length-derived target.** Removing the overloads breaks
the read path unless they are rerouted first:

- `field_size_deduce.hpp:22` — `struct_fields[field_size_type{}]`. This is
  *definitionally* a length target: it is how a `len_from_field` size is
  resolved while reading. Every length-prefixed vector/string in the suite goes
  through it.
- `computation_from_fields_impl.hpp:30` and `:61` — a `predicate` or
  `size_from_fields` callable naming a field that happens to be a length
  target. Line 30 is inside `is_invocable`, i.e. inside a *concept*, so the
  failure would not even be a clean error: the concept would silently go false
  and the constrained `operator()` would drop out of the overload set.
- `type_deduction/type/type_impl.hpp:34` — reads a *discriminant*, which stays
  visible. Unaffected.

So 043 must first factor the lookup that `operator[]` performs into a named
internal accessor that answers "the value of field `id` in this list",
unconditioned by the visibility policy, and route `deduce_field_size` and
`compute_impl` through it. `operator[]` then becomes policy plus a call to
that accessor, which keeps the two from drifting — the same argument the
existing code makes for having one `is_derived_field`. Put it next to
`operator[]` in `field_list.hpp` (both need `lookup_field` + `meta::type_of`),
name it for what it is (`field_value_of<accessor>(list)`), and give it a
comment saying it deliberately bypasses the visibility policy because the
library's own read path is not the user's model of the struct.

This also reframes 043's acceptance criterion about rewriting round-trip tests
and doc examples: they lose the ability to *read* a length field, and the
substitute is the data field's own `.size()`.

---

## 4. Slice 044 — value-based internals behind today's spelling

The trick that makes "no schema outside `include/` changes" achievable:
**every spelling the user writes today becomes a variable template that
evaluates to the new value.**

```cpp
template <std::size_t N>            inline constexpr auto fixed           = byte_count{N};
template <fixed_string id>          inline constexpr auto len_from_field  = field_accessor<id>{};
template <auto f, field_name_list ids> inline constexpr auto size_from_fields = size_from_fields_t<f, ids>{};
template <auto f, field_name_list ids> inline constexpr auto len_from_fields  = size_from_fields<f, ids>;
inline constexpr auto size_dont_care = size_dont_care_t{};
template <auto... sizes>            inline constexpr auto size_choices    = size_choices_t<sizes...>{};

template <auto s> inline constexpr auto field_size = s;   // 044-only: identity on an already-value size
```

Consequences:

- `field_size<fixed<4>>` parses as `field_size<byte_count{4}>` and evaluates to
  `byte_count{4}`. Token-for-token, every existing schema in `test/`, `docs/`
  and `examples/` keeps compiling — *because the wrapper it spells is now a
  no-op on values rather than a type constructor*. `field_size<len_from_field<"len">>`,
  `field_size<len_from_fields<f, with_fields<"a","b">>>` and
  `field_size<size_dont_care>` all fall out the same way. *Verified.*
- The alias parameters change kind (`fixed_size_like size_type` → `auto size`
  with a requires), which is invisible at the call site.
- `with_fields` / `fixed_string_list` are untouched in 044 — they are still the
  *type* argument of `size_from_fields_t`.
- 045 deletes `field_size` and `fixed` outright; `byte_count`/`_B` replaces the
  latter, and `size_dont_care`, `len_from_field`, `size_from_fields`,
  `len_from_fields` and `size_choices` survive as the values above.

### The one file outside `include/` that 044 cannot leave alone

`test/internals/traits_ct.cpp:35-41` asserts the *shape being replaced*, not
behaviour:

```cpp
expect(eq(s2s::is_variable_size_v<s2s::field_size<s2s::len_from_field<"hello">>>, true));
expect(eq(s2s::is_fixed_size_v<s2s::field_size<s2s::fixed<4>>>, true));
expect(eq(s2s::is_fixed_size_v<int>, false));
expect(eq(s2s::field_size<s2s::fixed<6>>::size_type_t::count, std::size_t{6}));
```

Lines 1-2 pass a *value* to a trait; lines 3-4 pass a *type* and reach into
`::size_type_t::count`. No choice of trait parameter kind satisfies both
halves, and nothing preserves `size_type_t` once the count lives in a value.
This block must be rewritten in 044 to
`is_fixed_size_v<decltype(field_size<fixed<4>>)>`-shaped assertions (cv-stripped)
plus a `deduce_field_size<2_B>{}() == 2`.

I am recording this as a **deviation from 044's acceptance criterion "No file
outside `include/` is modified by this issue"**, not as a reason to redesign.
What that criterion protects is the evidence: that the suite proving behaviour
was not rewritten alongside the thing it proves. Every *schema-spelling* file —
all of `test/schema/`, `test/doc_examples/`, `examples/`, `docs/`,
`test/shipped_header/`, and the rest of `test/internals/` including
`field_list_metadata_ct.cpp` — stays byte-identical, which is the evidence that
matters. Five lines of a white-box trait test necessarily move with the trait
they inspect. Line 98's `s2s::field<"y", float, s2s::field_size<s2s::fixed<4>>, ...>`
keeps working unchanged, since the shim hands `field` a value.

`single_header/s2s.hpp` is regenerated in 044 as well as 045 — otherwise
`shipped_header` builds the pre-044 tree. The spec only calls it out for 045;
treat it as required whenever `include/` moves.

---

## 5. Slice 045 — the new surface

### The option pack and its diagnostic

```cpp
template <typename S, typename T> concept size_option_like =
  fixed_size_like<S> || variable_size_like<S> || size_dont_care_like<S> || selectable_size_like<S>;

template <typename C, typename T> concept constraint_option_like =
  requires (const C& c, const T& v) { { c(v) } -> std::same_as<bool>; };

template <typename O, typename T> concept field_option_like =
  size_option_like<O, T> || constraint_option_like<O, T>;

template <fixed_string id, integral T, field_option_like<T> auto... opts>
  requires field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
using basic_field = field<id, T, size_of_pack<T, opts...>, constraint_of_pack<T, opts...>>;
```

Two things about this are load-bearing and were chosen by measuring the actual
compiler output, not by taste.

**The classification concept is applied as a placeholder constraint on the
pack, never as a fold in a requires-clause.** `field_option_like<T> auto...
opts` constrains each element separately; `requires (field_option_like<opts, T>
&& ...)` constrains the fold. gcc's output for the two is materially
different. Folded:

```
note: the expression '(field_option_like<opts, T> && ...) [with opts = {42}; T = int]' evaluated to 'false'
```

— it names the fold and dumps the whole pack, and says nothing about what an
option could have been. Per-element:

```
note: constraints not satisfied
required for the satisfaction of 'field_option_like<auto [requires field_option_like<<placeholder>, T>], T>'
  [with auto [...] = int; T = short unsigned int]
note: no operand of the disjunction is satisfied
   concept field_option_like = size_option_like<O, T> || constraint_option_like<O, T>;
                               ~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

— it isolates the offending entry, names its type, names the field type, and
prints both things it could have been. That is the "states what was expected"
the spec asks for, and it is why the concepts take the option's *type* as their
first parameter: a placeholder constraint `C<Args...> auto x` substitutes
`decltype(x)` first, so a value-first concept cannot be used this way at all.
Both forms *verified* on gcc 14; the `must_not_compile` case for 045 should
pin the per-element form.

The remaining gap: gcc names the entry's *type* (`= int`), not its position in
the pack. Position is not expressible in a concept. Do **not** buy it with a
`static_assert` inside the selector — the pack constraint fails first, at the
alias, so a static_assert deeper in would never fire. Accept type-naming; it is
unambiguous in practice because a schema author's bad entry is a literal they
can see.

**Duplicates need a `static_assert`, and that is the sanctioned fallback.** A
concept can classify an entry but cannot count how many entries classified the
same way; `basic_field<"v", u16, 2_B, 4_B>` must not silently take the first.
Put `static_assert(count_of_sizes <= 1, "...")` and its constraint twin inside
`size_of_pack`/`constraint_of_pack`, which are reached only after the per-entry
concept has passed.

`size_of_pack` / `constraint_of_pack` are `if constexpr` scans over the pack
returning the first entry of the matching kind, or the default
(`byte_count{sizeof(T)}` / `no_constraint<T>{}`) when there is none. Order
independence is exactly this scan; nothing else is needed. *Verified*, in
constant evaluation, for all four arrangements (neither option, size only,
constraint only, both in either order).

Which aliases get a pack: `basic_field`, `fixed_array_field`,
`array_of_records`, `fixed_string_field`, `c_arr_field`, `c_str_field`,
`vec_field`, `vector_of_records`, `str_field`, `struct_field`. `magic_*`
aliases fix the constraint themselves and take only a size. Aliases whose size
is determined by the type (`fixed_array_field`, `c_str_field`, …) accept
constraints only — spelling a size there is a contradiction and should stay
unspellable.

### `with_fields` elision

`with_fields<ids...>` becomes an `inline constexpr auto` of type
`fixed_string_list<ids...>`, and the *user-facing aliases that take a name
list* — `predicate`, `parse_if`, `eval_bool_from_fields`,
`eval_size_from_fields`, `size_from_fields`, `len_from_fields` — take
`auto... ids` and normalize:

```cpp
template <auto... ids> using field_names_of =
  /* one fixed_string_list value → its type; a pack of fixed_strings → fixed_string_list<ids...> */;

template <auto callable, auto... ids> using parse_if = compute<callable, bool, field_names_of<ids...>>;
```

Both `parse_if<f, with_fields<"a">>` and `parse_if<f, "a">` then work, which is
045's "both spellings accepted during the change", and the wrapped form dies
with the migration. Crucially **`compute`'s own parameter stays a
`field_name_list` type**, so every internal pattern match on
`compute<callable, bool, fixed_string_list<req...>>` — in
`field_list_metadata.hpp` (three of them), `computation_from_fields_impl.hpp`,
`clause_traits` — is untouched. Normalizing at the alias rather than at
`compute` is what keeps that blast radius at zero.

### Inside `variance`

Spelling-only, as the spec requires. `as_trivial<float, 4_B>`,
`as_vec<u32, len_from_field<"len">>`, `match_case<v, tag>` unchanged,
`type_switch`/`branch`/`predicate` unchanged apart from the name-list
normalization above.

One asymmetry to record because it looks like an oversight otherwise:
**`len_from_field<id>` becomes a value while `match_field<id>` stays a type
alias for the same `field_accessor<id>`.** `match_field` appears as a *type*
argument of `type<match_field<id>, sw>` and is pattern-matched as such by
`discriminant_obligation`, `extract_type_deduction_dependencies` and
`extract_switch_discriminants`; `type`'s first parameter also accepts
`compute<...>` and ladders, so it cannot become an NTTP. Two names over one
accessor template, one type-valued and one value-valued, is the cheapest
correct answer.

---

## 6. Decisions, stated explicitly

**State and data lifecycle.** This work introduces no persistent state, no I/O
and no partial-completion boundary: every construct here is consumed during
translation. The one lifecycle-shaped question it *does* touch is the
`struct_field_list`'s in-memory model, and 043 narrows it — a length-derived
slot stops being observable at all rather than being observable-but-stale. That
is a strengthening of an existing invariant (the writer overwrites those slots
unconditionally), not a new one. Nothing becomes irreversible, replayable or
auditable, and no read/write ordering changes: `is_dependencies_resolved`
still enforces that a size source precedes its dependent.

**Error propagation.** Two layers, and the seam between them is where a
diagnostic changes from a compile-time one to a runtime one. Above the seam
(schema declaration) every failure is a constraint failure on a named concept
at a user-facing alias, and no `std::expected` is involved. Below it
(read/write) nothing changes: `rw_result` / `cast_result` as today, and the
spec's Non-Goal on `cast_error` holds — no new `error_reason`, and
`found_contradicting_length` keeps its exact meaning because
`size_from_fields`'s sources stay visible and assignable. The one place the two
nearly meet is `is_derived_target`'s `static_assert` ("a derived length field
must be a fixed-sized field"), which stays a static_assert: it is a schema
error surfaced from the write machinery, and rephrasing it as a concept would
move it away from where the shape is known.

**Concurrency and ownership.** None. Every construct is either a compile-time
entity or a plain member of a `struct_field_list` owned by the caller;
`struct_field_list` is not thread-safe today and this work neither makes it so
nor assumes it. No shared mutable state, no statics with dynamic
initialization — the new `inline constexpr` variable templates are constant-
initialized, which also matters for the constexpr tier. Stated rather than
assumed, because the internal accessor of 043 hands out references into the
list and someone will eventually ask what synchronizes them: nothing, by
design; the list is a value.

**Reuse.** Reused rather than reimplemented: `field_accessor<id>` becomes the
`len_from_field` value verbatim (it is already exactly "a named field
reference", and `"len"_f` already produces it); `fixed_string_list` /
`field_name_list` stay the name-list carrier; `dep_vec`, `remove_duplicates`,
`flatten` and `static_set` carry the second derived-id list with no new
container; `no_constraint<T>{}` sets the precedent this design leans on for
"empty structural type as an NTTP"; `s2s_literals` already exists and gains
`_B` beside `_f` rather than a new namespace. Worth factoring *out* of this
work for the next comparable job: the internal field accessor of 043 (any
future code needing a field's value without the visibility policy), and
`field_names_of<...>` (any future alias taking a name list gets elision for
free).

**Extension points.** Deliberately none for third parties. The spec's
Non-Goals rule out a caller-defined size or constraint kind, so
`size_option_like` enumerates a closed set by explicit specialization rather
than by an open protocol — adding a kind means editing `field_size.hpp`, which
is the intended cost. The two places a *future maintainer* extends are
therefore known and small: a new size kind is a new empty structural type plus
one `is_*_size` specialization plus one `deduce_field_size` partial
specialization; a bit-denominated width is a `bit_count`/`_b` sibling of
`byte_count`/`_B` and the trait vocabulary already has room for it.

**Build vs. buy.** Nothing here is buyable. The candidates considered:
`std::integral_constant`/`std::ratio` for the width (rejected: puts the width
back in the type, and `_B` would need a literal-operator template — see
Alternatives); Boost.Hana or Boost.MP11 for the option-pack scan (rejected:
the project's own override says hand-rolled vocabulary types in `include/lib/`
specifically to keep template errors readable, the scan is nine lines of
`if constexpr`, and a header-only library with no dependencies is a stated
property); `std::tuple` as the option carrier (rejected: an `auto...` pack is
already the carrier, and a tuple would add a type to diagnose through).

**Abstractions introduced.** Five, each with the problem forcing it:
`byte_count` — an NTTP needs a structural type and a bare `std::size_t` would
be indistinguishable from any other integral option in the pack, so the
classification could not work; `size_from_fields_t`/`size_dont_care_t`/
`size_choices_t` — the same, one type per kind because the *type* is what the
traits match on (section 1); `field_option_like` and its two sub-concepts — the
pack has no positional meaning left, so kind is the only thing left to classify
by, and the diagnostic is a delivered requirement; `field_names_of<...>` — the
one normalizing metafunction, forced by "accept both spellings during the
change" without touching `compute`; the internal field accessor — forced by
section 3's finding that the library reads through the API it is about to
close. Explicitly *not* introduced: a `size_option` base class or CRTP (the
traits do the classifying), a `normalize<T>` type-level metafunction per alias
(the variable templates *are* the normalization, and they cost one line each),
and any wrapper preserved "for symmetry".

**Alternatives rejected.** Section 7.

---

## 7. Alternatives rejected

**Width in the type (`fixed<N>{}` as the value, `_B` as a literal-operator
template over a char pack).** Would have let every trait *and* `deduce_field_size`
stay type-keyed, which is the smaller diff. Rejected because `_B` then needs a
digit-parsing literal operator template, because it keeps one distinct type per
distinct width (the template-bloat property the rewrite is a chance to drop),
and because `byte_count{n}` is what generic internal code (`N * sizeof(T)`)
wants anyway. The cost of the choice is precisely one construct —
value-keyed constrained partial specializations of `deduce_field_size` — and
that was verified on all three available compilers before committing to it.

**Value-keyed traits (`template <auto v> concept fixed_size_like`).** Reads
more directly at the call site (`fixed_size_like<size>` rather than
`fixed_size_like<decltype(size)>`). Rejected because a value-parameterised
concept cannot be a placeholder constraint, so the option pack would have
needed a fold — and section 5 shows the fold's diagnostic is materially worse.
Keeping one type-keyed vocabulary also preserves `is_fixed_size_v<int> ==
false` as a meaningful question.

**Folding the classification concept in a requires-clause.** Rejected on
measured output alone.

**One tagged `derived_field_ids` list.** Rejected in section 3: needs an
ordering over the tagged entry, cannot represent a field that is both kinds
without duplication, and buys nothing the two lists don't give.

**Making `len_from_field` targets const-only rather than absent.** That is
what the code already does, and the spec rejects it explicitly; recorded here
because it is the natural thing to re-propose when the read path breaks
(section 3) and the temptation will be to soften 043 rather than add the
internal accessor.

**Keeping `field_size<>` as a deprecated alias past 045.** Rejected by the
spec (single atomic migration, no external users). Recorded because the 044
shim *is* that alias, and it would be easy to leave it standing; deleting it is
part of 045's definition of done.

**Normalizing at `compute` rather than at the name-list aliases.** Would make
`compute`'s third parameter an `auto...` pack and force rewrites of all five
internal pattern matches on `compute<callable, R, fixed_string_list<...>>`.
Rejected: same user-visible result, strictly larger blast radius.

---

## 8. Findings surfaced by this design, out of its scope

**A `variance` with two alternatives sharing a C++ type is broken today, and
the positional variant is not the reason.** `utils/helper.hpp:47` builds
`std::variant<type_from_type_condition_v<cases>...>` with duplicates permitted,
and the index-based inversion in `discriminant_obligation::value_at` and the
`std::get<idx>` in the write path are both fine with duplicates. The read path
is not: `field_reader.hpp:236` does `variant = std::move(field.value)` — a
*converting* assignment, which for `std::variant<u32, u32>` has no unique
best alternative and is ill-formed. So a schema such as
`match_case<0, as_trivial<u32, 2_B>>` beside `match_case<1, as_trivial<u32, 4_B>>`
— the same C++ type at two wire widths, which is exactly what per-alternative
`size_choices` exists to express — fails to compile at the reader, not at the
variant. No test in the tree covers it (`field_list_metadata_ct.cpp`'s union
cases all use distinct types), which is why it has gone unnoticed.

This design does not fix it and must not: the fix is `variant.template
emplace<idx>(...)` in `read_variant_impl` (the write side already indexes),
and that touches the read path this spec declares untouched. It is worth its
own issue. Note that the fix is *cheap* and that 045 makes the broken schema
easier to write by making per-alternative widths easier to spell, so filing it
before 045 lands is the prudent order.

**`union_field` passes an unwrapped `size_dont_care` where every other site
passes `field_size<size_dont_care>`** (`field.hpp:97`). Under today's traits
`is_size_dont_care_v` is therefore false for a union field's own size — inert,
because `union_field` has its own `extract_length_dependencies` specialization,
but it is a latent trap. The value representation collapses the distinction and
the inconsistency disappears for free; no action needed beyond not
reintroducing it.

**`is_selectable_size_v` reads `is_fixed_size<T>::res` (issue 027).** The
re-derived trait should read `is_selectable_size<T>::res`, because writing a
knowingly-wrong new trait is not defensible and the old spelling is being
deleted anyway. This is behaviour-neutral: nothing in `include/` consumes
`selectable_size_like` or `is_size_like` today. It also does **not** close 027,
because what actually makes `size_choices` undeclarable is that no user-facing
alias accepts a selectable size — `basic_field` requires fixed, `vec_field`
requires variable. 027 stays open and pending, retargeted at declarability
rather than at the trait body. Flagging the judgement call: if the human reading
this would rather the trait keep its wrong body until 027 is scheduled, that is
a one-line change to this plan.

---

## 9. Sequencing and verification

044 and 043 are independent (043 changes no spelling); 045 depends on 044.
Recommended order 044 → 043 → 045, so that the atomic migration in 045 rewrites
schemas that already have their final `operator[]` behaviour — 043 landing last
would mean touching doc examples twice.

Per slice, the bar is the same and comes from `AGENTS.md`: `ctest` green
tree-wide *including* every `*_compile_time` and `*_coverage` entry.
`ct_coverage_check.cmake` is the real gate here — a construct that quietly is
not constant-evaluable shows up as a compile-time/run-time count mismatch, not
as a failure, and the coverage build is what catches it. Every construct this
design introduces was prototyped as `static_assert`s precisely so that gate is
met by construction rather than discovered: value-shaped sizes are
constant-initialized `inline constexpr` variables, the pack scans are
`if constexpr` over NTTPs, `deduce_field_size` keeps its `constexpr operator()`,
and the internal accessor of 043 is the same `static_cast` chain `operator[]`
already performs in constant evaluation.

New tests the slices owe:

- 044: the rewritten `traits_ct.cpp` block (section 4), plus `deduce_field_size`
  over each size kind. No other test changes.
- 043: `test/must_not_compile/` case naming a length target (must fail as
  no-such-member, distinct from the existing `derived_field_assignment.cpp`,
  which must keep failing as assign-to-const for a *discriminant*).
- 045: `test/must_not_compile/` case for an unrecognized pack entry, pinning
  the per-element diagnostic; `doc_examples_match` and `shipped_header` after
  `scripts/amalgam.py` regeneration.

## 10. Risks and open questions

- **MSVC 19.39 is unverified.** `AGENTS.md` claims it as supported and there is
  no build CI (only `docs.yml`), so nothing would catch a regression there. The
  construct at risk is the constrained partial specialization of
  `deduce_field_size` with an argument list identical to the primary's. If it
  turns out unsupported, the fallback is a single primary with `if constexpr`
  over `decltype(size)` and two `operator()` overloads — uglier, no capability
  lost. Worth deciding whether MSVC's claim in `AGENTS.md` is still meant to be
  true.
- **`size_choices` shape.** The design keeps `size_choices_t<vals...>` as
  internal plumbing for `union_field` and re-derives `is_selectable_size`
  honestly. Whether that shape survives contact with a *declarable*
  `size_choices` is exactly what 027 is left open on.
- **Diagnostic on clang and MSVC.** The measured output in section 5 is gcc 14.
  clang's concept diagnostics differ in wording; the per-element form is
  structurally better on both, but the `must_not_compile` case should assert on
  the build failing, not on the message text, so the test stays portable.
- **The 044 acceptance-criterion deviation** (section 4) needs the human's
  agreement, since it is stated in the issue as an absolute.
