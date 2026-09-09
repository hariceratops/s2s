# Design: byte order is data, not a template parameter

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md` (frozen).
Issues served, in order:
`dev/issues/053-runtime-byte-order-in-the-read-path.md`,
`dev/issues/054-a-top-level-announcing-record-sets-the-order.md`,
`dev/issues/055-ladder-and-callable-announcement-forms.md`,
`dev/issues/056-reject-order-dependent-fields-in-an-announcing-record.md`,
`dev/issues/057-an-announcing-record-nested-below-the-top-level.md`,
`dev/issues/058-wrong-entry-point-is-a-compile-error.md`,
`dev/issues/059-the-stored-marker-decides-the-order-on-write.md`,
`dev/issues/060-reject-more-than-one-announcing-record.md`,
`dev/issues/061-document-the-byte-order-axis.md`.

Cut from `main` at `f311009`. Sits on top of
`dev/design/union-alternatives-have-no-option-pack.md` (048–052),
`dev/design/schema-api-verbosity.md` (043–045) and
`dev/design/unbounded-resize-from-wire-length.md` (046–047), and obeys their
governing rules verbatim:

- **A class-type NTTP is a non-deduced context in a partial specialization**, so
  anything a trait pattern-matches on lives in the value's *type*. This design
  never asks the compiler to deduce a value out of a class-type NTTP.
- **`field/field.hpp` keeps its three-header include closure.** The new field
  kind added there names its deduction guide as an opaque `typename` and
  includes nothing new — the same discipline that produced
  `type_condition_list` in 048.
- **Concepts, not SFINAE; `std::expected`, never codes; trailing return types
  everywhere; both a `ut` and a GoogleTest tier for anything touching a stream.**

This design proposes **three deviations from the issues as frozen**, each argued
in place and collected in §12: 055's "pull the names into the dependency table"
is not implementable as written (§3.6); an announcement reached only
conditionally or repeatedly is rejected, which no issue enumerates (§5.2); and
053 cannot land without a float-capable byteswap, which no issue mentions
(§2.3).

---

## 0. The shape in one page

| Piece | Where | What |
|---|---|---|
| `cast_endianness&` as a runtime argument | `struct_cast_impl`, `stream_cast_impl`, every `read_field`/`write_field`, `read_impl`/`write_impl` | replaces the `auto endianness` template parameter everywhere below the entry point |
| `deduce_byte_order(std::endian)` | `stream/byte_order.hpp` | runtime overload beside the existing template; the one place `std::endian` collapses to `cast_endianness` |
| `byteswapped<T>` | `stream/byte_order.hpp` | **new**; `std::byteswap` for integrals, `bit_cast`-round-trip for floats. Forced by 053, see §2.3 |
| `order_case`, `order_branch`, `order_switch`, `order_if_else`, `order_from` | **new** `include/order_deduction/` | the declaration surface, mirroring `match_case`/`branch`/`type_switch`/`type_if_else`/`type` one for one |
| `deduce_order<guide>` | **new** `include/order_deduction/order_deduction_impl.hpp` | reuses `evaluate_switch_helper` and `evaluate_ladder_helper` **verbatim** and maps the returned index to a declared `std::endian` |
| `order_announcing_field<base_field, guide>` | `field/field.hpp` | the new field kind, shaped exactly like `maybe_field` — public inheritance from the base field plus two member aliases |
| `announces_byte_order<id, record, guide, opts...>` | `api/field_descriptors.hpp` | the descriptor alias, shaped exactly like `variance` |
| `read_field` / `write_field` specialization | `field_read/field_reader.hpp`, `field_write/field_writer.hpp` | reads/writes the base record, then resolves. **The two folds are not touched by this feature** |
| the census and the order-agnostic walk | **new** `include/field_list/announcing_record.hpp` | one depth-first walk feeding 056, 058 and 060 |
| `struct_cast<T>(s)`, `stream_cast<T>(s, obj)` | `api/struct_cast.hpp`, `api/stream_cast.hpp` | new entry points; the four existing ones gain the complementary constraint |
| seeding `field::value` from an `eq` constraint | `field/field.hpp` | what makes the frozen-marker composition *fall out* instead of failing (§7.2) |

Slice split, unchanged from the issues:

- **053** — the order becomes a runtime value. No schema change, no API change,
  no test edit. The existing suite is the proof — with the one exception in
  §2.3, which is an addition rather than an edit.
- **054** — `order_announcing_field`, `announces_byte_order`, the `match_field`
  form of `order_from`, `struct_cast<T>`, the reader.
- **055** — the ladder and `compute_t` forms.
- **056** — the order-agnostic predicate and the walk it rides on.
- **057** — nesting. **Needs no new mechanism** (§5.1); the slice is its tests
  plus the off-spine rejection (§5.2).
- **058** — the two read entry-point constraints and their diagnostics.
- **059** — the write path, `stream_cast<T>`, the two write entry-point
  constraints, and the frozen composition.
- **060** — the `<= 1` assertion, riding the census 056 built.
- **061** — documentation.

---

## 1. The one invariant everything else follows from

**Resolution is a property of stream position, not schema-tree position.**
Every other decision here is a consequence, so it is worth naming the structure
that makes it true rather than a rule the code has to remember:

> The byte order is a single mutable cell, owned by the entry point, borrowed by
> reference through the entire recursion, and written exactly once — at the
> instant the announcing record's last field leaves the stream.

Because the reference is threaded *down* into nested casts and the cell lives
*above* them, a write performed at depth 4 is visible to the depth-3 fold's next
sibling, to the depth-2 fold's next sibling, and to everything after. The
"escapes to the rest of the file" semantics is not implemented by a rule; it is
what a reference to a caller-owned cell already does. Nothing propagates the
order back up a return value, so there is no path by which the two directions of
travel could disagree.

The corollaries, all of which show up later:

- **The fold does not need to know about announcements.** The cell is updated by
  the announcing field's *reader*, which is the code that runs at exactly the
  stream position the rule names. `struct_cast_impl` and `stream_cast_impl` keep
  the shape 053 leaves them in (§3.4).
- **The check on the announcing record's own fields is a check about a stream
  prefix**, not about a subtree of the schema. §4.3 is where that bites.
- **Write mirrors read for free**, because the write path has the same fold in
  the same order over the same fields (§7.1).

---

## 2. Slice 053 — the order becomes a runtime value

### 2.1 The signature change, exhaustively

The template parameter `auto endianness` disappears from everything below the
entry points and reappears as a `cast_endianness&` function argument. The
complete list, because a missed forward declaration here is a link-time-shaped
puzzle rather than a compile error:

| Site | Today | After |
|---|---|---|
| `cast/struct_cast_impl.hpp:13,16` | `struct_cast_impl<F, stream, endianness>` | `struct_cast_impl<F, stream>`; `operator()(stream&, cast_endianness&)` |
| `cast/stream_cast_impl.hpp:13,16` | `stream_cast_impl<F, stream, endianness>` | `stream_cast_impl<F, stream>`; `operator()(stream&, const S&, cast_endianness&)` |
| `field_read/field_reader.hpp:169` | forward decl of `struct_cast_impl`, 3 params | 2 params |
| `field_write/field_writer.hpp:99` | forward decl of `stream_cast_impl`, 3 params | 2 params |
| every `read_field<…>::read` (7) | `template <auto endianness, typename stream> read(stream&)` | `template <typename stream> read(stream&, cast_endianness&)` |
| every `write_field<…>::write` (7) | same shape | same shape |
| `read_buffer_of_records::read`, `read_variant_impl::read`, `read_variant_helper::read` | `template <auto endianness, …>` | runtime arg |
| `write_variant_impl::write`, `write_variant_helper::write`, `write_nested`, `verify_then_write` | `template <auto endianness, …>` | runtime arg |
| `field_read/read_impl.hpp:125` | `read_impl<endianness, ceiling>(s, obj, N)` | `read_impl<ceiling>(s, obj, N, order)` |
| `field_write/write_impl.hpp:87` | `write_impl<endianness>(s, obj, N)` | `write_impl(s, obj, N, order)` |
| `api/struct_cast.hpp`, `api/stream_cast.hpp` | pass `std::endian::little`/`big` as an NTTP | collapse to a `cast_endianness` local and pass it by reference |

Two decisions inside that table:

**The argument is `cast_endianness&` at every level, including the leaves that
can only read it.** A leaf reader has no business writing the cell, and a
by-value `cast_endianness` would say so. Uniformity wins anyway: one signature
for `read` across all seven specializations means `read_buffer_of_records` and
`read_variant_helper` forward it without a per-kind decision, and the two folds
are written once. The alternative — by value at the leaves, by reference at the
composites — puts a rule in the reader's head about which kind it is forwarding
to, which is exactly the sort of rule that is wrong after the next field kind is
added. `read_impl`/`write_impl` themselves take it **by value**, because they
are the bottom and have nothing to forward to.

**`ceiling` stops being "the defaulted NTTP after endianness".** 047 placed it
second precisely so existing call sites kept compiling; with endianness gone it
becomes the only template parameter with a default, and the two call sites in
`field_reader.hpp` (`read_impl<bound_in_bytes<T::field_bound>>(…)` and the bare
`read_impl(…)`) read better than they do today. No behaviour change.

`struct_cast_le` becomes:

```cpp
template <fixed_order_schema T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_le(stream& s) -> std::expected<T, cast_error> {
  auto order = deduce_byte_order<std::endian::little>();
  return struct_cast_impl<T, stream>{}(s, order);
}
```

The `fixed_order_schema` constraint is 058's; in 053 the parameter stays
`field_list_like`.

### 2.2 The leaf, made total

Today's foreign arm has no `else` and is sound only because it is never
instantiated for a `T` that is neither `trivial` nor `buffer_like`. The
partition is worth writing down, because it turns out the two arms already cover
the same set:

| arm | admits |
|---|---|
| host | `variable_sized_buffer_like` (= `vector_like ∪ string_like`) → `read_native<ceiling>`; otherwise `constant_sized_like` (= `fixed_buffer_like ∪ trivial`) → `read_native` |
| foreign | `trivial` → `read_foreign_scalar`; `buffer_like` (= `fixed_buffer_like ∪ variable_sized_buffer_like`) → `read_foreign_buffer` |

`trivial ∪ fixed_buffer_like ∪ vector_like ∪ string_like` on both sides. So
"make the foreign arm total" is not "find the missing case" — it is "say so".
The outer `if constexpr` becomes a runtime `if/else`, and the foreign arm's
inner ladder gains a final `else` holding a `static_assert` on a dependent false
naming the type. For a `T` outside the partition the host arm today fails as *no
viable overload of `read_native`*; after this change both arms fail, one of them
with a sentence. The inner `if constexpr`s are otherwise untouched, per 053.

**Instantiating both arms for every `T` was checked case by case and holds.**
`std::string` under foreign: `read_native<ceiling>` then `byteswap_elements`,
which loops elements of size 1 and swaps nothing. `fixed_string<N>`:
`byteswap_elements` returns immediately on `fixed_string_like`. `char[N+1]`:
range-for over a C array, elements of size 1. `std::array<u8, N>`: same. None of
these is reachable today under `struct_cast_le` on a little-endian host, and all
of them compile.

### 2.3 The finding that makes or breaks 053: `std::byteswap` has no float overload

`std::byteswap` is constrained to integral types. `byteswap_elements`
(`stream/byte_order.hpp:22`) calls it on any element with `sizeof > 1`, and
`read_foreign_scalar`/`write_foreign_scalar` call it on any `trivial T` — and
`trivial` is `floating_point<T> || integral<T>`.

Today that is latent: a float field only reaches the foreign arm when the caller
picks the entry point whose order differs from the host's. **After 053 both arms
are instantiated for every field type, in both directions**, so a schema
containing a float stops compiling under *any* entry point.

This is not hypothetical. `test/schema/union_read.cpp` declares
`s2s::as_trivial<float, 4_B>` in seven schemas and reads them with
`struct_cast_le` (lines 79, 128, 187, 243, 294, 344, 446, 490);
`test/internals/field_list_metadata_ct.cpp` has three more. Every one of them
breaks the moment 053 lands, against 053's own criterion that no existing test
is edited.

The fix is small and belongs in 053:

```cpp
template <trivial T>
constexpr auto byteswapped(T v) -> T {
  if constexpr(std::is_integral_v<T>)
    return std::byteswap(v);
  else {
    static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                  "a floating-point field is byte-swapped through an unsigned "
                  "integer of the same width; this type has no such width and "
                  "no portable wire form");
    using rep = uint_of_size_t<sizeof(T)>;
    return std::bit_cast<T>(std::byteswap(std::bit_cast<rep>(v)));
  }
}
```

`std::bit_cast` is `constexpr`, so the constexpr-stream tier keeps working.
`byteswap_elements`, `read_foreign_scalar` and `write_foreign_scalar` call
`byteswapped` instead of `std::byteswap`. `long double` is rejected with a
sentence rather than silently mis-swapped — an 80-bit extended has no wire
representation to swap into, and no format this library targets carries one.

**Rejected: excluding floats from the foreign arm with a `static_assert`.** It
would make 053 compile and make `struct_cast_be` on a float field stop
compiling, which is a regression in the shipped feature set.

**Rejected: leaving it to a follow-up issue.** 053's acceptance criterion is
that the existing suite passes unmodified; there is no version of 053 that meets
it without this.

### 2.4 What 053 costs

Honestly, because the PRD asks for the claim and not a benchmark. The brief's
argument — "one leaf instantiation holding both arms replaces two holding one
each" — is true only of a program that casts the *same schema in both
directions*. For the common case, a program that only ever calls
`struct_cast_le`:

- **Below the leaf:** each `read_impl<T>` instantiation now contains the foreign
  arm as well as the host one. That is real growth. At `-O1` and above the order
  is a local initialized from a `constexpr` call and the dead arm folds away
  after inlining; at `-O0` it does not.
- **Above the leaf:** unchanged. One `struct_cast_impl<F, stream>` where there
  was one `struct_cast_impl<F, stream, little>`.

For a program casting one schema both ways, everything above the leaf halves,
which is where the depth is. The requirement is qualitative and this is the
qualitative answer: single-direction programs pay a little at the leaf,
dual-direction programs save a lot above it, and no numeric budget is added.

---

## 3. The declaration surface (054, 055)

### 3.1 A field kind, not an option-pack entry

**Decision: `order_announcing_field<base_field, guide>`, a field kind in
`field/field.hpp` shaped exactly like `maybe_field`, spelled by a descriptor
alias `announces_byte_order` shaped exactly like `variance`.**

```cpp
// field/field.hpp — includes nothing new; `guide` is opaque here
template <typename base_field, typename order_deduction>
struct order_announcing_field : base_field {
  using field_base_type = base_field;
  using byte_order_deduction = order_deduction;
};
```

```cpp
// api/field_descriptors.hpp
template <fixed_string id, field_list_like T, order_deduction_like guide,
          constraint_option_like<T> auto... opts>
using announces_byte_order =
  order_announcing_field<field<id, T, size_dont_care, constraint_of_pack<T, opts...>>,
                         guide>;
```

The PRD leaves this open and names the alternative: an entry in the record's
trailing option pack, following 048–052. That alternative was worked through and
rejected, for three reasons in increasing order of weight.

**The line 048–052 actually drew is not "options are the new default".** It is:
a *value the leaf consumes* rides the pack (size, constraint, bound); a
*behaviour the engine performs* is a field kind (`maybe`, `variance`). 052
rejected a wrapper for the union-level constraint precisely because a constraint
is a value consumed by a check, not a new behaviour. An announcement is the
other side of that line: it consumes nothing at the leaf and changes what
happens after the field. Following 048–052's *rule* lands on a field kind;
following its *shape* would land on a pack. The rule is the part worth
inheriting.

**The pack route needs a sixth NTTP on `field`, and the blast radius has silent
holes.** An option pack resolves into `field`'s parameter list, so the
announcement would have to be a sixth NTTP. A defaulted sixth parameter does not
keep five-parameter partial specializations matching — they stop matching
entirely. That is 18 sites: six in `field_traits.hpp`, one in
`field_metafunctions.hpp`, two in `field.hpp` (`to_optional_field`,
`no_variance_field`), seven in `field_list_metadata.hpp`, plus
`to_field_choices` and the descriptor aliases. Four of those seven metadata
metafunctions (`extract_parse_dependencies`,
`extract_type_deduction_dependencies`, `extract_unconditional_len_sources`,
`extract_switch_discriminants`) have catch-all primaries returning an empty
`dep_vec`, so a missed specialization is not a compile error — it is a schema
whose length dependency silently disappears. 047 paid this cost once for
`bound`; it was worth it for a value every field kind can carry. It is not worth
it for a marker at most one field in a schema carries.

**The wrapper adds nothing to the most-instantiated template in the project.**
The compile-time-neutrality goal is qualitative but not nothing, and a sixth
NTTP on `field` is paid by every field in every schema.

What the wrapper costs, in full — eight additions, no modifications to existing
pattern matches:

1. `is_order_announcing_field` + `order_announcing_field_like` in
   `field_traits.hpp`, matching
   `order_announcing_field<field<id, T, size, constraint, bound>, guide>` with
   `field_list_like T`.
2. A disjunct in `field_like`, so `struct_field_list` accepts it.
3. `read_field` and `write_field` specializations (§3.4, §7.1).
4. An `extract_length_dependencies` specialization forwarding to
   `field_base_type` — **required**, because that metafunction is declared
   without a primary definition and a wrapper that reaches no specialization is
   a hard error. The forwarded base is a `size_dont_care` field, so the answer
   is the empty vector; the specialization exists to say so out loud.
5. The four catch-all metadata metafunctions are **correct by falling through**,
   and this is the one place that needs stating rather than assuming: an
   announcement's field names resolve inside the *inner* record's table, so the
   containing list genuinely has no dependency to record (§3.6).
6. The census (§4.2).
7. The descriptor alias.
8. Documentation.

`struct_field_like<order_announcing_field<…>>` is false — the wrapper is not a
`field<…>` — so `read_field`'s concept dispatch has no ambiguity to resolve, and
the inherited `field_id`, `field_type`, `constraint_checker` and `value` reach
`generate_field_table`, `operator[]` and both folds unchanged.

**Rejected: marking the record's *field list* rather than the field.** It reads
better — a marker record type that is inherently the announcer, reusable across
schemas — and it dies on `struct_field_list_impl<metadata, fields...>` being
pattern-matched in six places (`field_value_of`, three `deduce_type`
specializations, `evaluate_ladder`, `compute_impl`, both cast impls). Adding a
parameter before the pack breaks all of them; deriving a new list type from it
matches none of them.

### 3.2 The guide, and what it actually reuses

`include/order_deduction/`, mirroring `include/type_deduction/` one construct
for one:

```cpp
template <auto v, std::endian order>
struct order_case { static constexpr auto value = v;
                    static constexpr auto byte_order = order; };

template <evaluates_to_bool eval, std::endian order>
struct order_branch { using expression = eval;
                      static constexpr auto byte_order = order; };

template <order_case_like... cases>   requires (sizeof...(cases) > 0) struct order_switch  { … };
template <order_branch_like... brs>   requires (sizeof...(brs)   > 0) struct order_if_else { … };

template <typename... Args> struct order_from;              // mirrors `type`
// order_from<match_field<"id">, order_switch<…>>
// order_from<compute_t<callable, R, names>, order_switch<…>>
// order_from<order_if_else<…>>
```

A TIFF header, which is the shape acceptance is judged against:

```cpp
using order_marker = s2s::struct_field_list<
  s2s::fixed_array_field<"marker", u8, 2>
>;

using tiff_header = s2s::struct_field_list<
  s2s::announces_byte_order<"byte_order", order_marker,
    s2s::order_from<s2s::match_field<"marker">,
      s2s::order_switch<
        s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
        s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
  s2s::magic_number<"magic", u16, 2_B, 42>,
  s2s::basic_field<"ifd_offset", u32>
>;
```

**What is reused, and it is the part that is code.** `evaluate_switch_helper`
(`switch_impl.hpp:22`) and `evaluate_ladder_helper` (`ladder_impl.hpp:31`) are
declared over *unconstrained* packs and touch only `::value` and `::expression`
respectively. They fold to a `type_deduction_idx`, and an index is exactly what
a positional case list resolves to on either axis. `deduce_order` calls them
unchanged and maps the index through `std::array{cases::byte_order...}`.
`compute_t`, `compute_impl` and `can_eval_R_from_fields` are reused verbatim for
the callable form. Roughly forty lines of evaluation logic are shared; what is
new is five empty carrier structs and the index-to-order lookup.

**What is not reused, and why not.** `match_case<v, T>` and `branch<eval, T>`
require `type_tag_like T`, and `type_switch`/`type_if_else` expose
`using variant = variant_from_type_conditions_v<cases...>` — a *member typedef*,
instantiated with the class, which requires every case's `type_tag::type` to
exist. Sharing the carriers therefore needs one of two things, both worse than a
parallel four-line struct:

- **Give an order outcome a `::type`** so it satisfies `type_tag_like`. It would
  work, and it would also make
  `variance<"x", type<match_field<"m">, type_switch<match_case<1, as_order<…>>>>>`
  a schema that compiles into a `std::variant<std::endian, std::endian>`. A
  spelling that is nonsense and accepted is worse than a spelling that does not
  exist.
- **Make `variant` lazy** so a case list without type tags does not force it.
  `union_field` reads `type_deducer::variant` eagerly in its own base-class
  list, so the laziness would have to survive into `field.hpp` — machinery in
  the hottest template in the project, to avoid five empty structs.

The mirroring is also the user-facing argument: a schema author who knows
`variance<"body", type<match_field<"tag">, type_switch<match_case<…>>>>` can
read `announces_byte_order<…, order_from<match_field<"marker">,
order_switch<order_case<…>>>>` without being told anything.

### 3.3 The schema speaks `std::endian`; the engine speaks `cast_endianness`

`cast_endianness` is `{host, foreign}` — relative to the machine, and therefore
not something a schema author can write. `II` means little-endian on every
machine. So:

- `order_case`/`order_branch` carry a `std::endian`, the same vocabulary
  `struct_cast_le`/`struct_cast_be` already pass at the entry point.
- `deduce_order` returns `std::expected<std::endian, error_reason>`.
- `stream/byte_order.hpp` gains a **runtime overload**
  `constexpr auto deduce_byte_order(std::endian) -> cast_endianness` beside the
  existing template. The template stays: the fixed entry points still collapse a
  compile-time constant. The overload is required because a resolved order is a
  runtime value by construction.

One collapse point, two callers, no new vocabulary type. A schema that writes
`std::endian::native` gets the machine's order and is legal but useless; nothing
special-cases it.

### 3.4 Resolution happens in the reader, not in the fold

**Decision: `read_field<order_announcing_field_like T, F>` does the resolution.
`struct_cast_impl`'s fold is not touched by this feature at all.**

```cpp
template <order_announcing_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;  F& field_list;

  template <typename stream>
  constexpr auto read(stream& s, cast_endianness& order) const -> rw_result {
    using base = typename T::field_base_type;
    auto res = read_field<base, F>(field, field_list).read(s, order);
    if(!res)
      return res;
    auto resolved = deduce_order<typename T::byte_order_deduction>{}(field.value);
    if(!resolved)
      return std::unexpected(resolved.error());
    order = deduce_byte_order(*resolved);
    return {};
  }
};
```

Three things this buys, none of which the fold-side alternative does:

- **The update happens at literally the stream position §1 names.** The base
  reader returns when the record's last field leaves the stream; the next
  statement writes the cell. There is no window in which a field is read at a
  stale order.
- **Nesting needs nothing.** At depth, this reader is reached through
  `read_field<struct_field_like>` → `struct_cast_impl` → the inner fold, all of
  which already forward `order` by reference. §5.1.
- **The two folds stay identical to their 053 form.** An
  `if constexpr(announcing…)` in `struct_cast_impl`'s fold lambda would be a
  second place that knows about this feature, and it would have to be mirrored
  in `stream_cast_impl` — the drift 059 explicitly worries about.

`read_field<base, F>(field, field_list)` binds `T&` to `base&` through public
inheritance; the base is `field<id, inner_list, size_dont_care, constraint>`, so
it dispatches to the `struct_field_like` specialization and recurses into
`struct_cast_impl<inner_list, stream>`. The same `order` reference goes down.
That is harmless rather than merely tolerable: the inner list is guaranteed to
contain no announcement (§8) and every one of its fields is guaranteed
order-agnostic (§4), so nothing down there can read or write the cell to any
effect. **Rejected: passing a local copy down into the announcing record** to
make "parses at a fixed order" structurally true — it is defensive code for a
state the compile-time checks already exclude.

### 3.5 A marker that matches nothing

054: "fail as an ordinary constraint failure — a `cast_error` carrying
`error_reason::validation_failure` and the field's id. Nothing special is added
for this case."

`deduce_order` returns `std::unexpected(error_reason::validation_failure)` when
the fold yields no index. The reader propagates it as an `rw_result`; the
enclosing fold attaches `fields::field_id`, which is the announcing *field's*
id. So a TIFF file starting `XX` fails as
`cast_error{validation_failure, "byte_order"}`.

**Rejected: `error_reason::type_deduction_failure`**, which is what
`evaluate_switch` raises on the type axis and would have been free. On the type
axis, "deduction failed" describes a genuine inability to proceed: the reader
does not know how many bytes to consume next or what to consume them into. On
the order axis it describes a file that is not this format — semantically a
magic mismatch, which is `validation_failure` everywhere else in this library. A
reader who sees `type_deduction_failure` will go looking for a `variance` field
that does not exist. No new enumerator either way, which is what "nothing
special is added" means.

### 3.6 Where the announcement's field names are checked — and 055's criterion

The names in `match_field<"marker">` or `compute_t<f, R, names>` resolve inside
the **announcing record's own field table**, one level below the list that
declares the announcement.

055 asks that they be "pulled into the dependency table through the existing
`extract_type_deduction_dependencies` path, not a parallel one". **As written
that is not implementable, and following it would break.**
`is_dependencies_resolved` (`field_list_metadata.hpp:362`) looks every
dependency up in *this list's* `field_table` and dereferences the result:

```cpp
auto dep_field_info = field_table[dep_field];
auto dep_field_idx  = dep_field_info->occurs_at_idx;
```

A name that is a field of the inner record is not in the outer table, so
`field_table[dep_field]` returns an empty `static_optional` and the `->` is a
constant-evaluation error at every schema declaration. The criterion presumes
the names live in the same scope the table indexes, and the PRD's own scoping
rule says they do not.

**What is reused instead, which is the substance of the criterion.** The
*extraction* metafunction shape is copied one for one:
`order_deduction_dependencies<guide>` with three specializations — one for
`order_from<match_field<id>, …>`, one for
`order_from<compute_t<callable, R, fixed_string_list<req…>>, …>`, one for
`order_from<order_if_else<branches…>>` reusing `extract_req_fields_from_clause`
and `remove_duplicates` verbatim — producing the same `dep_vec`. What changes is
the consumer: instead of a `dependency_table_t` entry checked by
`is_dependencies_resolved`, the vector is checked for *existence only* against
the inner list's `field_table`, in the same `announcing_record_check` that hosts
056's and 060's assertions (§4.2).

Existence is the whole obligation, and this is the reason the ordering half has
nothing to do: **every field of the announcing record is read before resolution
runs**, so no name can be "not yet read". The ordering check that
`is_dependencies_resolved` exists to perform is vacuous on this axis. That is
not a shortcut; it is the stream-position rule again.

The diagnostic:

```
"the byte-order announcement names a field the announcing record does not have;
 names resolve inside the announcing record's own field list, not in the record
 that contains it"
```

Noted while confirming this: the *type* axis has the weaker version of the same
check. `deduce_type<type<match_field<id>, _switch>>` reaches the field through
`sfl[field_accessor<id>{}]`, so a misspelled discriminant fails as *no viable
`operator[]`* rather than as a sentence, whereas the `compute_t` form gets
`field_value_of`'s `static_assert`. Finding 5.

---

## 4. Slice 056 — the order-agnostic check and its exact boundary

### 4.1 The predicate

A field is order-agnostic when reading it produces the same bytes in the cell's
either state. On the value type:

| type | agnostic when |
|---|---|
| `trivial T` | `sizeof(T) == 1` |
| `fixed_string_like T` | always — `byteswap_elements` returns immediately on it, and `write_foreign_buffer` routes it to `write_native` |
| `fixed_array_like`, `is_c_array_v` | the element type is |
| `vector_like` | `value_type` is |
| `string_like` | always (`char`) |
| `field_list_like` | every field of it is — this is where the walk descends |

On the field, so that the wrapper kinds are covered:

| field kind | reduces to |
|---|---|
| `fixed_sized_field_like`, `variable_sized_field_like` | the value type, above |
| `struct_field_like` | every field of `extract_type_from_field_v<T>` |
| `array_of_record_field_like`, `vector_of_record_field_like` | every field of the element list |
| `optional_field_like` | `field_base_type` |
| `union_field_like` | every alternative in `field_choices` |
| `order_announcing_field_like` | cannot occur — §8 rejects it first |

A variable-length field's *length* is a different field of the same record and
is checked on its own, which is the right answer: `str_field<"s",
len_from_field<"n">>` inside an announcing record is agnostic in its payload and
its `u16` length source is not, so the record is rejected — correctly, because
that `u16` is read before the order is known.

This is what makes 056's "byte arrays and other order-agnostic spellings of a
marker are accepted, including a magic declared as bytes rather than as an
integer" true by construction: `magic_byte_array<"marker", 2, …>` is
`std::array<unsigned char, 2>`, elements of size 1, agnostic;
`magic_number<"marker", u16, 2_B, 0x4949>` is a two-byte integer and is
rejected.

### 4.2 The walk, and where it is anchored

**Decision: one depth-first census in a new header
`include/field_list/announcing_record.hpp`, sitting above `field_list.hpp` and
below `api/`, consumed by `api/struct_field_list.hpp` and the four entry
points.**

Not in `field_list_metadata.hpp`, and the reason is an include cycle rather than
taste: descending from a `struct_field` into its inner record's fields means
pattern-matching `struct_field_list_impl<metadata, fields...>`, which is
declared in `field_list.hpp`, which includes `field_list_metadata.hpp`. A
forward declaration would break the cycle, but the count has no consumer inside
the metadata either — every consumer (`create_struct_field_list`, the four entry
points) is at or above `api/`. Placing it above `field_list.hpp` costs nothing
and keeps the metadata header's closure as it is.

The census carries two counts rather than one:

```cpp
struct announcement_census { std::size_t on_spine; std::size_t off_spine; };
```

per field: an `order_announcing_field_like` field contributes one to `on_spine`;
a `struct_field_like` field contributes its inner list's census unchanged; an
array/vector of records, a `maybe`, or a union alternative contributes its inner
census **with both counts folded into `off_spine`**; anything else contributes
nothing. §5.2 is what the second count is for.

`create_struct_field_list` then hosts one more check struct beside
`dependency_check`, following that established pattern exactly — a struct of
`static_assert`s whose `res` a concept reads, so the failure arrives as both a
sentence and an unsatisfied constraint:

```cpp
template <typename... fields>
struct announcing_record_check {
  static constexpr auto census = census_of_fields<fields...>::value;
  static_assert(census.on_spine + census.off_spine <= 1,  /* 060's message */);
  static_assert(census.off_spine == 0,                    /* §5.2's message */);
  // for the one announcement declared directly in this list, if any:
  static_assert(all_fields_order_agnostic,                /* 056's message */);
  static_assert(announcement_names_resolve,               /* §3.6's message */);
  static constexpr bool res = …;
};
```

The order-agnostic and name-resolution assertions are *local*: they concern the
announcement this list declares directly, and they are checked at the level that
declares it. The census assertions are *cumulative* and therefore checked at
every level, which costs nothing and means the outermost list catches a pair of
announcements that individually sit in different subtrees.

060's "the detection uses the same depth-first walk 056 introduced, not a second
traversal" is met exactly: one census, three consumers.

### 4.3 The boundary the check does **not** have, and why that is a hole

056 fixes the scope: the announcing record's own fields, not the record that
contains it, and a test proves an order-dependent *later* sibling of the
container compiles and reads correctly. That is implemented as written.

**It leaves the earlier siblings uncovered, and they are read before the order
is known.** In

```
outer:
  a:   u32                  <- read before resolution, at the seed order
  hdr: <announcing record>
  b:   u32                  <- read after resolution, correct
```

`a` is not a field of the announcing record, so 056's check does not see it, and
it is read at whatever the cell was seeded with. For `struct_cast<T>` that is
`cast_endianness::host` — an arbitrary value, chosen because 056 guarantees
nothing can observe it. `a` observes it.

The PRD's normative sentence is "Every field that is read *before* the order is
known … must be order-agnostic"; the apposition that follows identifies that set
as "the announcing record's own fields", which is true exactly when nothing
precedes the announcement in stream order. In TIFF, pcap and ELF nothing does —
`II`/`MM` is at byte 0, pcap's magic is at byte 0, and ELF's `EI_DATA` at byte 5
is preceded only by `\x7fELF` and two `u8`s, all agnostic. So the shipped check
is correct for every format the feature targets and wrong for a schema nobody
has written yet.

**This design implements 056's scope as frozen and records the gap rather than
silently widening it.** The strengthening is a strict superset — restrict every
field visited up to and including the announcement in depth-first stream order —
it passes all of 056's and 057's criteria including the must-not-over-reach one
(later siblings stay unrestricted), and it rejects only schemas that are already
wrong. It is additive and can land later without breaking anything. Finding 3,
and a question for the user in the report accompanying this design.

Until it lands, 061's documentation states the rule in its true form: a field
read before the announcement completes is read at an unspecified order, and the
library currently checks only the announcing record's own fields.

---

## 5. Slice 057 — nesting

### 5.1 Why it needs no new mechanism

Given §1 and §3.4, 057 is a test slice. Walk the case it exists to prove:

```
outer:
  container:            <- struct_field
     x: bytes[2]        <- announcing record's sibling? no: see below
     hdr: announcing    <- resolves here
     y: u32             <- LATER SIBLING OF THE CONTAINER: must use the resolved order
  z: u32                <- after the container closes: must use the resolved order
```

`read_field<struct_field_like>` for `container` calls
`struct_cast_impl<container_list, stream>{}(s, order)` — passing the *same*
reference. Inside, the fold reads `hdr`, whose reader writes the cell, and then
reads `y`, which reads the cell. Control returns to the outer fold, which reads
`z` from the same cell. Nothing forwards anything back up; the cell was never
copied. Depth is irrelevant, so "more than one level deep" is the same code
path.

"The order is resolved exactly once" is structural rather than enforced: there
is exactly one announcing field in the schema (§8), it is on the unconditional
spine (§5.2), and its reader is the only writer of the cell below the entry
point. There is no `resolved` flag, because there is nothing for one to guard.

### 5.2 The rejection 057 forces and no issue enumerates

"Resolved exactly once" is a load-bearing invariant, and three schema shapes
break it:

- an announcement inside a `maybe` — the cast can finish without ever resolving,
  and everything after reads at the seed order;
- an announcement inside an `array_of_records` or `vector_of_records` — resolved
  once per element, so the last element silently wins;
- an announcement inside a union alternative — resolved only if that alternative
  is selected.

All three are counted into `off_spine` by the census and rejected:

```
"a byte-order-announcing record must be read exactly once, unconditionally.
 This one sits inside an optional, an array or vector of records, or a union
 alternative, so a cast could finish without ever resolving an order, or could
 resolve one repeatedly. Move the announcement onto the unconditional path."
```

This is not in 056's or 060's criteria. It is not scope creep either: without
it, 057's own criterion that the order is resolved exactly once is a property
the library states and does not hold. Flagged in §12 as a deviation so the user
can strike it if they would rather ship the hole.

---

## 6. Slice 058 — the entry points

Four entry points exist today (`struct_cast_le`, `struct_cast_be`,
`stream_cast_le`, `stream_cast_be`) and two are added (`struct_cast`,
`stream_cast`). 058's criteria name only the read pair; the write pair gets the
identical treatment in 059, because `stream_cast_le` on a self-announcing schema
would otherwise silently write in an order the marker contradicts — the exact
failure 058 exists to prevent, one direction over.

Two concepts, each backed by an assertion-bearing struct so the failure carries
wording, following `dependency_check`'s established shape:

```cpp
template <typename T>
struct self_announcing_check {
  static constexpr auto count = announcement_count_of<T>::value;
  static_assert(count != 0,
    "struct_cast is for a schema that declares a byte-order-announcing record — "
    "one whose bytes decide the order, like TIFF's II/MM. This schema declares "
    "none, so its byte order is a fixed fact about the format: call "
    "struct_cast_le or struct_cast_be instead.");
  static constexpr bool res = (count == 1);
};

template <typename T>
concept self_announcing_schema = field_list_like<T> && self_announcing_check<T>::res;

template <typename T>
struct fixed_order_check {
  static constexpr auto count = announcement_count_of<T>::value;
  static_assert(count == 0,
    "struct_cast_le and struct_cast_be are for a schema whose byte order is "
    "fixed by the format. This schema declares a byte-order-announcing record, "
    "so the file decides its own order: call struct_cast instead. There is "
    "deliberately no way to force a fixed order over a self-announcing schema.");
  static constexpr bool res = (count == 0);
};

template <typename T>
concept fixed_order_schema = field_list_like<T> && fixed_order_check<T>::res;
```

with the write-side messages naming `stream_cast` / `stream_cast_le` /
`stream_cast_be`. Evaluating the concept instantiates the struct, which fires
the assertion — the same mechanism `all_dependencies_resolved` uses, so this is
an existing idiom rather than a new trick. The four existing entry points swap
`field_list_like T` for `fixed_order_schema T`; the 67 files calling them are
unaffected, since a schema with no announcement satisfies it.

`count >= 2` never reaches here: §8 rejects it at schema declaration, which is
strictly earlier and names a better problem.

`struct_cast` itself:

```cpp
template <self_announcing_schema T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast(stream& s) -> std::expected<T, cast_error> {
  auto order = cast_endianness::host;   // the seed; §4.3 is the only way to observe it
  return struct_cast_impl<T, stream>{}(s, order);
}
```

**The caller-supplied-runtime-order non-goal stays open.** It would arrive as a
third overload taking a `std::endian` argument, on a third concept, without
touching the engine — the cell is already runtime and already seeded at the
entry point. Nothing here forecloses it.

---

## 7. Slice 059 — the write path

### 7.1 The mirror

```cpp
template <order_announcing_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  template <typename stream>
  constexpr auto write(stream& s, cast_endianness& order) const -> rw_result {
    using base = typename T::field_base_type;
    auto res = write_field<base, F>(value, field_list).write(s, order);
    if(!res)
      return res;
    auto resolved = deduce_order<typename T::byte_order_deduction>{}(value);
    if(!resolved)
      return std::unexpected(resolved.error());
    order = deduce_byte_order(*resolved);
    return {};
  }
};
```

Line for line the read reader with `read` swapped for `write`. `value` *is* the
inner field list — `field_type` is inherited from the base — so the same
`deduce_order` instantiation serves both directions.

**Decision: resolve after the record is written, not before the cast begins.**
The PRD observes that the value is available upfront, and it is; resolving
upfront would need a depth-first *search* for the announcing field at write
time, and a second rule about when the resolved order starts applying. Since the
announcing record's own fields are order-agnostic (§4), the bytes are identical
either way, so the choice is entirely about which rule the code states.
Resolving at the point of writing states the same rule the read path states, in
the same place, which is 059's own stated goal: "one rule stated once, so the
two paths cannot drift."

`stream_cast<T>(s, obj)` mirrors `struct_cast<T>(s)`, seeding the same cell.
Everything §5.1 says about nesting holds verbatim, because
`write_field<struct_field_like>` → `write_nested` → `stream_cast_impl` forwards
the same reference.

The marker keeps its `operator[]` setter: the announcing field is not derived
(nothing derives it — the census is not a derivation table), not a length
target, not a discriminant, and not frozen unless the *inner* marker field
carries an `eq`. `is_frozen_field` asks `frozen_field_like<meta::type_of<…>>` of
the wrapper, which needs `fixed_sized_field_like`, which is false for it. So
setting `fl["byte_order"_f]["marker"_f]` and observing the output order change
works with no new machinery — 059's criterion.

### 7.2 The `eq{v}` composition, and why it does *not* fall out today

The PRD asks the design to show why the frozen composition falls out rather than
asserting it. Worked through, **it does not** — and the reason is one line in
`field.hpp`.

Constrain the marker: `fixed_array_field<"marker", u8, 2, eq{std::array<u8,2>{'I','I'}}>`.
`frozen_field_like` holds (`fixed_sized_field_like` ✓, `is_eq_constraint_v` ✓,
convertible ✓), so `frozen-fields-write-their-own-value` removes the setter and
`write_field` emits `frozen_value_of<T>` (`field_writer.hpp:52`). The stored
`field.value` is never assigned by anyone. `field`'s member is `field_type
value{}` (`field.hpp:29`), so on a default-constructed struct it is `{0, 0}`.

Now `stream_cast` on that struct: the announcing writer calls `deduce_order`,
which reads the marker through `operator[]` or `field_value_of` — both of which
hand back the *stored* value. `{0, 0}` matches no `order_case`, and the write
fails with `validation_failure`. 059's criterion says "no setter, one order, **no
error**".

**Decision: seed `field::value` from an `eq` constraint at construction.**

```cpp
// field/field.hpp
static constexpr auto initial_value() -> field_type {
  if constexpr(is_eq_constraint_v<std::remove_cvref_t<decltype(constraint_on_value)>> &&
               std::is_convertible_v<decltype(constraint_on_value.v), field_type>)
    return static_cast<field_type>(constraint_on_value.v);
  else
    return field_type{};
}
field_type value{initial_value()};
```

The condition is `frozen_field_like`'s two interesting conjuncts, inlined
because `frozen_field_like` lives in `field_traits.hpp`, which includes
`field.hpp`. Dropping its `fixed_sized_field_like` conjunct costs nothing: `eq`
over a `std::string` or `std::vector` is not spellable as an NTTP, and `eq` over
a C array fails the convertibility test — which is the same oddity
`frozen_field_like`'s own comment already documents.

With that, the composition genuinely falls out and can be stated in one
sentence: **a frozen field's stored value is the value it writes.** The
resolution reads the stored value like any other field, gets `v`, and resolves
one order forever. Nothing about either feature special-cases the other. The
absence of a setter stops mattering because there is nothing left to set.

**This is a behaviour change to a shipped feature, deliberately.** A
default-constructed `struct_field_list` now reads back its magic fields as their
magic values instead of as zeros. That is more truthful than what it does today,
and `frozen_value_of`/`is_frozen_target_v` stay exactly as they are — the write
path keeps taking the value from the constraint, so the two can never disagree.

**Rejected: an `effective_value_of` accessor** — `frozen ? frozen_value_of<F> :
field_value_of<…>` — used by a write-side flavour of `deduce_order`. It works
for the `match_field` form in one line, and then needs `compute_impl` to gain a
reader-policy parameter so the callable and ladder forms see the same values,
and it leaves the general problem in place: **every write-side computation that
reads a frozen field sees the stale default today** — `maybe`'s presence
predicate (`field_writer.hpp:194`), a computed size, a ladder branch. Seeding is
six lines and fixes all of them; the accessor is more code, fixes one, and would
make the composition something this feature arranges rather than something that
falls out.

**Rejected: rejecting `eq` on an announcing record's marker.** The PRD names the
composition as legitimate-if-degenerate and asks for it to work.

---

## 8. Slice 060 — more than one announcement

`static_assert(census.on_spine + census.off_spine <= 1, …)` in
`announcing_record_check`, at every list construction, so a schema with
announcements in two different subtrees is caught at the outermost list that
contains both.

The diagnostic has to say *limitation*, not *conflict*, per 060:

```
"a schema may declare at most one byte-order-announcing record. This is a
 current limitation of s2s, not an inherent conflict: a second announcement
 taking over from the first follows from the same stream-position rule at no
 extra cost, and is left undecided only until a real format settles whether the
 second overrides the first and whether nesting is restricted. Merge the two
 announcements into one record, or read the second region as a separate cast."
```

Checked at schema declaration rather than at the cast, because a schema with two
announcements is wrong whether or not anybody casts it, and because at the cast
it would surface as *both* entry-point concepts failing, which reads as a
library bug.

Rejecting it here also keeps the deferred subtree-scoped case (Exif MakerNotes)
open: an opt-in "this announcement governs only its own subtree" is a second
parameter on `order_announcing_field` or a second field kind, and neither is
foreclosed by anything above. 061 documents the two deferrals together, as 060
asks.

---

## 9. Decisions, stated explicitly

**State and data lifecycle.** One piece of state, and it is the whole feature:
the `cast_endianness` cell. It is owned by the entry-point function, lives on
its stack, is borrowed by `&` through the recursion, and dies when the cast
returns. It is written exactly once per cast and read by every leaf. Nothing is
persistent, nothing is cached across casts, and two concurrent casts share
nothing because each has its own local. **Partial completion is observable and
is not made reversible**, in either direction and deliberately: a read that
fails after resolution leaves a discarded `struct_field_list` the caller throws
away, and a *write* that fails after resolution leaves bytes on the stream that
cannot be recalled — but that is the pre-existing property of every s2s write
(`stream_cast_impl` has always emitted field by field), and this feature adds no
new failure between the announcement and the end. The one ordering guarantee
that matters is the one §1 states: the cell is written before any field after
the announcement is touched, and never after. Nothing here is replayable or
auditable and nothing needs to be; the schema is the audit trail.

**Error propagation.** Unchanged in mechanism, one new seam and one deliberate
narrowing. The new seam is `deduce_order`, which speaks
`std::expected<std::endian, error_reason>` and raises
`error_reason::validation_failure` when the marker matches nothing (§3.5). It
sits inside the announcing field's reader/writer, which speak `rw_result` like
every other reader, and the enclosing fold widens to `cast_error` and attaches
the field id exactly as it does for a constraint failure. No layer learns
anything about the layer below: `deduce_order` knows a field list and a case
list and does not know the announcing field's name; the fold supplies it. The
narrowing is that a resolution failure *inside a nested* announcing record is
reported at the outermost record field's id, because
`read_field<struct_field_like>` already discards the inner `cast_error`'s
`failed_at` (`field_reader.hpp:185`) — pre-existing, applies to every nested
failure, not widened here. No new `error_reason` enumerator; three compile-time
rejections that would otherwise have been runtime errors (§4, §5.2, §8) are
`static_assert`s instead, which is the whole point of them.

**Concurrency and ownership.** Single-threaded, no shared mutable state, and
this is worth stating rather than assuming because the feature introduces the
library's *first* piece of mutable state that outlives the field being read.
The cell is a function-local owned by one entry-point invocation; every other
reference to it is a borrow with a strictly shorter lifetime. No statics, no
thread-locals, no `mutable` members, no caching of a resolved order between
casts. Two threads casting the same schema from different streams share nothing
— including nothing constexpr-adjacent, since every metafunction result is a
type or a `constexpr` value. The property that would break silently if someone
later hoisted the cell somewhere longer-lived — say, caching "this schema
resolved to big-endian last time" — is that a stale order would be applied to a
file that never announced one, and every compile-time check here would still
pass. There is deliberately no place to hoist it to.

**Reuse.** Used rather than reimplemented: `evaluate_switch_helper` and
`evaluate_ladder_helper` verbatim, which is where the actual evaluation logic
lives and which cost nothing to share because 048 left both packs unconstrained;
`compute_t`, `compute_impl` and `can_eval_R_from_fields` for the callable form;
`extract_req_fields_from_clause`, `flatten` and `remove_duplicates` for the
ladder's name extraction; `dependency_check`'s assert-struct-behind-a-concept
shape for all five new compile-time rejections; `maybe_field`'s public-inheritance
shape for the new field kind and `variance`'s alias shape for its descriptor;
`deduce_byte_order` as the single collapse point from `std::endian` to
`cast_endianness`; `frozen_value_of` and `is_frozen_target_v` untouched (§7.2
supplies a better stored value to a mechanism that already worked).
Introduced for reuse: `byteswapped` is the project's byte-swap from now on and
is where the next type with no `std::byteswap` overload gets handled once;
`announcement_census` is one traversal with three consumers and is where a
fourth whole-schema question about announcements gets asked without a fourth
walk; `order_deduction/` mirrors `type_deduction/` closely enough that a third
"deduce something from parsed fields" axis has a template to copy.

**Extension points.** Three, and one explicit non-extension.
(1) **A caller-supplied runtime order** (a Non-Goal) is a third overload on a
third concept, seeding the same cell — nothing in the engine changes, which is
what "must not be shaped in a way that rules it out" required.
(2) **Subtree-scoped resolution** (the Exif case, a Non-Goal) is a second
parameter on `order_announcing_field` or a sibling field kind; the reader would
save and restore the cell around its own subtree instead of writing through it.
The cell being a borrowed reference rather than a global is what makes that a
five-line change rather than a redesign.
(3) **A second announcement taking over from the first** (§8) is deleting one
`static_assert` once the semantics is settled; the engine already does the right
thing, because the cell is a cell.
There is deliberately **no** extension point for a user-supplied resolution
callable outside `compute_t`, and none for a user-registered `order_case`
outcome type: `std::endian` has two useful values and `order_case` carries one
of them. `order_case_like` and `order_branch_like` are closed sets by explicit
specialization, exactly as `is_type_tag` is.

**Build vs. buy.** Nothing here is buyable, and the two places it looked like it
might be were checked. `std::byteswap` **is** the buy, and §2.3 is the precise
statement of what it cannot do — floating-point types, which `trivial` admits
and every existing float test exercises; the build is nine lines wrapping the
buy rather than replacing it. `std::endian` is bought outright as the schema's
vocabulary rather than introducing an s2s enum, which is why §3.3 has a collapse
function instead of a mapping table. Boost.Hana/MP11 for the census walk was
evaluated and rejected on the same grounds 045 rejected it — s2s has no
dependencies and the project's own override mandates hand-rolled vocabulary in
`include/lib/` to keep template errors readable — and the census is a fold over
a pack of `std::size_t` pairs, which is not where a metaprogramming library
would earn its keep. The genuinely expensive one to get wrong in the other
direction was the *type-deduction machinery*: building a second evaluator would
have been the mistake, and §3.2 buys it entirely and builds only the carriers it
cannot type-check.

**Abstractions introduced.** Six, each with the problem that forces it.
`order_announcing_field` — a field whose completion changes the engine's state
needs a distinct read/write step, and `read_field`'s dispatch is by concept over
the field type (§3.1). `order_case`/`order_branch`/`order_switch`/`order_if_else`
/`order_from` — counted as one: the type axis's carriers force a
`type_tag_like` outcome and eagerly instantiate a `std::variant` over
`type_tag::type`, so an order outcome cannot ride them without either a fake
`::type` that makes nonsense schemas compile or lazy `variant` machinery in the
project's hottest template (§3.2). `deduce_order` — the index-to-order mapping
has no home on the type axis, which maps an index to an alternative. `announcement_census`
— three whole-schema questions need one traversal, and 060 requires it to be one
(§4.2). `byteswapped` — §2.3. `include/field_list/announcing_record.hpp` — a
location rather than an abstraction, forced by `field_list.hpp` including
`field_list_metadata.hpp` (§4.2).
Explicitly **not** introduced: no `byte_order_state` struct wrapping the cell
(one `cast_endianness&` is the state, and a struct would be a type to diagnose
through with no second member to justify it); no `resolved` flag (§5.1 —
nothing to guard); no per-field endianness override (a Non-Goal, and nothing
here is shaped toward it); no new `error_reason` (§3.5); no policy parameter on
`compute_impl` (§7.2 solves it at the root instead); no separate write-side
`deduce_order` (§7.1).

**Alternatives rejected** — collected; each argued in place.
The announcement as an entry in the record's trailing option pack, with a sixth
NTTP on `field` (§3.1); the announcement on the record's *field list* type
rather than on the field (§3.1); sharing `match_case`/`branch` by giving an
order outcome a fake `::type` (§3.2); sharing them by making `type_switch::variant`
lazy (§3.2); an s2s enum for the declared order instead of `std::endian` (§3.3);
resolving in `struct_cast_impl`'s fold instead of in the reader (§3.4); passing
a copy of the cell into the announcing record to make its fixed order structural
(§3.4); `error_reason::type_deduction_failure` for a marker that matches nothing
(§3.5); routing the announcement's field names through the containing list's
dependency table as 055 asks (§3.6 — not implementable);
`static_assert`-ing floats out of the foreign arm, and deferring the float fix
to a follow-up issue (§2.3); `cast_endianness` by value at the leaves and by
reference at the composites (§2.1); resolving the write order upfront from the
struct instead of at the point the record is written (§7.1); an
`effective_value_of` accessor plus a reader policy on `compute_impl`, instead of
seeding the stored value (§7.2); rejecting `eq` on a marker (§7.2).

---

## 10. Tests

Routing is by constant-evaluability and the compiler enforces it
(`dev/specs/compile-time-test-tier.md`): anything holding a real stream is
GoogleTest under `add_struct_cast_test`, anything constant-evaluable is `ut`
under `add_ut_test`'s triple build. Test lambdas must not capture — schema
constants belong in the schema type or as `constexpr` locals — or `*_coverage`
catches it while the other two entries stay green. `single_header/s2s.hpp` is
regenerated in every commit, since `must_not_compile/` and `shipped_header/`
consume the amalgam.

**053 — no test edits, by criterion, and one addition that is not an edit.**
§2.3 means the slice must *prove* the float path, not merely not break it: a
`float` and a `std::array<float, 2>` field, round-tripped through both `_le` and
`_be`, in `trivial_read`/`trivial_write` and their `_ct` mirrors. Without it the
regression surfaces in `union_read.cpp` as a wall of template output naming
`as_trivial`, which is the wrong place to learn about `std::byteswap`. Land the
float cases **before** the rest of 053 so they fail loudly for the right reason.

**Added during scaffolding: `test/internals/announcement_census_ct.cpp`.**
The census is the load-bearing metaprogram behind 056, 058 and 060, and every
one of those slices' criteria is negative ("fails to compile"), which cannot
distinguish *counted correctly* from *counted wrongly in a way nothing observes
yet*. The positive form is `static_assert`s over `census_of_fields<…>::value`
for: a flat schema with none; one at the top; one nested two deep; one behind a
`maybe`, an `array_of_records` and a union alternative (each expecting
`off_spine == 1`); two in disjoint subtrees. Plus the order-agnostic predicate
asserted directly over each of the value-type rows in §4.1's table. Written and
passing before 054, so it is a description first and a regression gate
afterwards; verified non-vacuous by flipping one expected count.

**054** — `test/schema/byte_order_axis_read.cpp` + `_ct.cpp`. The TIFF-shaped
schema of §3.2 read in both the `II` and `MM` forms, asserting the `u16` magic
comes out as 42 both ways (the whole claim — the same four bytes decode
differently); a marker matching neither, asserting
`cast_error{validation_failure, "byte_order"}`; and one control asserting a
schema with no announcement still reads identically under `struct_cast_le`.

**055** — the same pair gains: a ladder form; a `compute_t` form over *two*
fields of the announcing record, which is the case a single `match_field` cannot
express and therefore the one that justifies marking the record rather than the
field; and a three-way equivalence — one schema expressed all three ways over
the same bytes, asserting identical output, which is 055's "all three forms
produce identical reads". `test/must_not_compile/byte_order_misuse.cpp` CASE 7,
a `match_field` naming a field the announcing record does not have.

**056** — `must_not_compile` CASE 3 (an order-dependent `u16` directly in the
announcing record) and CASE 4 (the same, nested one record deeper, which is what
proves the walk descends). The positive half is the one that matters more: in
`byte_order_axis_read`, an order-dependent later sibling of the *containing*
record, asserted to read correctly — the over-reach guard. Plus acceptance of a
`magic_byte_array` marker and of a `fixed_string` marker.

**057** — announcing record one level deep, then two; assertions on a later
sibling *of the container* and on a record read after the container closes, as
separate cases, because a single test covering both passes with the reference
threading half-broken. `must_not_compile` CASE 6 for §5.2's off-spine
rejection.

**058** — `must_not_compile` CASE 1 (`struct_cast` on a fixed-order schema) and
CASE 2 (`struct_cast_le` on a self-announcing one), each matched against its
expected diagnostic.

**059** — `byte_order_axis_write.cpp` + `_ct.cpp`: byte-for-byte round trip
through `struct_cast` and `stream_cast` in both forms; a case that *sets*
`fl["byte_order"_f]["marker"_f]` directly and observes the output bytes change,
which is the criterion that the marker keeps its setter; the nested case, since
write-side resolution at depth is a separate code path from read; the frozen
composition — an `eq`-constrained marker, asserting the write succeeds from a
**default-constructed** struct (which is precisely what fails without §7.2's
seeding, so this test is the proof of that decision); and `must_not_compile`
CASE 8 and 9 for the write entry points. Read and write stay separate files
rather than one round-trip, per the tree's convention and because a round-trip
stays green if both directions are wrong the same way — which, for a byte-order
feature, is the likely failure.

**060** — `must_not_compile` CASE 5 (two announcements in disjoint subtrees, so
it also proves the walk is cumulative rather than per-level), plus the
over-firing guard: the existing single-announcement cases at three depths
already serve as it, so nothing new is needed beyond naming it in the issue's
closing note.

**The `must_not_compile` cases are registered by the slice that implements
them, not at scaffolding time.** `add_rejected_case` sets `WILL_FAIL`, so a case
failing for the wrong reason passes the harness while testing nothing — the trap
047 fell into on CASE 4 and 048–052 documented. Every case is checked against a
no-`CASE` control build first, and each is confirmed to fail on its own
assertion rather than on an unrelated arity error.

---

## 11. Documentation (061)

The site owns the schema language, and today it says byte order is chosen by
which function the caller calls. Concretely:

- **New page `docs/schema/byte-order-axis.md`**, and a nav entry in
  `docs/mkdocs.yml` — `docs_nav_lists_every_page` fails otherwise. Contents: the
  announcing record, all three deduction forms, the stream-position rule stated
  once and in full (it governs the containing record's later siblings, not only
  what follows the container), the order-agnostic restriction with its boundary
  **and §4.3's honest caveat** that only the announcing record's own fields are
  checked, and both deferrals as limitations with the note that lifting either
  is additive.
- **`docs/schema/index.md`** — the descriptor table gains
  `announces_byte_order`, mirroring how `variance` appears.
- **`docs/reading.md:12`** — the sentence fixing byte order to the entry point
  becomes true-of-fixed-order-formats, with `struct_cast` beside it.
- **`docs/writing.md:12`** — same, plus the marker deciding the output order and
  being ordinary settable data rather than a frozen field.
- **`docs/reference.md`** — `announces_byte_order` in the declaration block, and
  the block gains the entry points, which it does not list today at all;
  `## Known limitations` gains the one-announcement and no-subtree-scoping
  entries.
- **`README.md:41,48`** — "in either byte order" and "Compile time endianness
  handling" both overstate now; the second is the one that becomes actively
  false.
- **`docs/errors.md`** — a marker matching no case is `validation_failure` at
  the announcing field's id, and why it is not `type_deduction_failure`.

Any complete program shown needs a `test/doc_examples/` source with the
`<!-- docs: … -->` / `// docs-begin` binding and a CTest target; a TIFF header
is the natural worked example and reads from a `std::ifstream` opened
`std::ios::binary` over a fixture-generated file, per the tree's rules. The PRD
does not require a TIFF example to ship, but 061 requires the worked examples it
shows to compile, so whatever the page shows is bound.

---

## 12. Findings and risks

1. **`std::byteswap` has no floating-point overload, and 053 makes that fatal.**
   §2.3. Ten existing test schemas contain a `float`. Fixed inside 053 rather
   than deferred, because 053's own criterion cannot otherwise be met. This is
   the highest-risk item in the feature and it is in the first slice.

2. **055's dependency-table criterion is not implementable as written.** §3.6.
   `is_dependencies_resolved` dereferences a lookup in *this* list's field
   table, and the announcement's names are fields of the inner list. The design
   reuses the extraction and checks existence against the inner table instead;
   the ordering half is vacuous on this axis. The issue text should be corrected
   before implementation, or the deviation accepted in review.

3. **The order-agnostic check does not cover fields read before the
   announcement in an enclosing record.** §4.3. Implemented as 056 freezes it.
   The strengthening — restrict every field up to and including the announcement
   in depth-first stream order — is a strict superset that still passes 056's
   over-reach guard and 057's criteria, and it rejects only schemas that are
   already wrong. Not taken here because 056 fixes the scope in writing.
   **This is the one place where following the issue leaves a silent wrong
   answer rather than a missing feature**, and it is the question raised with the
   user alongside this design.

4. **An announcement reached conditionally or repeatedly is rejected, which no
   issue enumerates.** §5.2. Forced by 057's own "resolved exactly once"
   criterion. Strikeable if the user would rather ship the hole; if struck, 057's
   criterion needs rewording, because it would no longer be true.

5. **The type axis's `match_field` form has no name-existence diagnostic.**
   Noticed in §3.6: `deduce_type<type<match_field<id>, _switch>>` reaches the
   field through `operator[]`, so a misspelled discriminant fails as *no viable
   overload* while the `compute_t` form gets `field_value_of`'s `static_assert`.
   Pre-existing, out of scope, worth its own issue — the order axis gets the
   good version here, which makes the asymmetry visible.

6. **Write-side computations over a frozen field see the stale default today.**
   §7.2. `maybe`'s presence predicate (`field_writer.hpp:194`), a computed size
   and a ladder branch all read a frozen field's zeroed storage rather than its
   frozen value. This design fixes the class at the root by seeding
   `field::value`, which is what makes 059's composition criterion true; the
   incidental effect is that those three stop being wrong too. Worth stating in
   review, because a fix to three unrelated paths arriving inside a byte-order
   slice is exactly the kind of thing a reviewer should be told about rather
   than discover.

7. **Seeding `field::value` changes observable behaviour of a shipped
   feature.** §7.2. A default-constructed `struct_field_list` now reads back its
   magic and `eq`-constrained fields as their values instead of zeros. No
   existing test was found asserting the old behaviour, but the search was by
   reading rather than by building.

8. **Almost nothing here was compiled.** Every claim about include direction,
   concept membership, the host/foreign partition (§2.2), and which
   metafunctions have catch-all primaries was verified by reading the tree.
   The two constructs that most deserve a prototype before 053 and 054 are
   opened: (a) `read_impl` instantiating **both** arms for each of
   `u8`, `u32`, `float`, `std::array<u32,4>`, `char[8]`, `fixed_string<4>`,
   `std::vector<u32>` and `std::string`, clean under
   `-Wall -Wextra -Wpedantic` on g++-14 and clang++-19; and (b)
   `read_field<order_announcing_field_like T, F>` binding `T&` to `base&` and
   dispatching to the `struct_field_like` specialization, which is the one place
   the wrapper's public inheritance has to do real work.

9. **MSVC 19.39 remains unverified**, as in all three prior designs. The
   construct most at risk is `order_case<v, std::endian>`'s class-type NTTP
   first parameter — `std::array<u8,2>{'I','I'}` as an NTTP argument, deduced as
   `auto v` and compared with `==` inside `evaluate_switch_impl`. It is the same
   shape `match_case<v, T>` already has and which the union tests exercise, so
   the risk is low, but there is still no build CI.

10. **`scripts/amalgam.py` needs no change** — it walks all of `include/` and
    topologically sorts by the include graph, so `include/order_deduction/` and
    `include/field_list/announcing_record.hpp` are picked up automatically
    (re-verified for this design). `include/s2s.hpp` is a hand-maintained
    roster and **does** need the new public headers added, or a consumer of the
    non-amalgamated tree gets `order_from` undeclared.

---

## 13. Decisions taken on §12's findings

Settled with the user on 2026-09-06, after this design was written. Recorded
here because §12 poses three of them as open and an implementer reading only
the design would otherwise have to guess.

**Finding 2 — 055's dependency-table criterion (§3.6): issue corrected.** The
criterion as frozen was not implementable; `dev/issues/055` has been amended to
match what §3.6 designs — reuse `extract_type_deduction_dependencies`, check
existence against the *inner* record's field table, and treat the ordering half
as vacuous on this axis. No deviation remains to accept in review.

**Finding 3 — the order-agnostic boundary (§4.3): ship 056 as frozen, document
the hole.** The check covers the announcing record's own fields only. A field
read before the announcement in an enclosing record is read at the seed order
and is not diagnosed. This is correct for every format the feature targets
(TIFF, pcap and ELF all put the marker at or near byte 0 with nothing
order-dependent ahead of it) and wrong only for a schema nobody has written.
The strengthening described in §4.3 is additive and can land later without
breaking anything; it is deliberately not taken now. 061 must state the rule in
its true form — see the criterion added there.

**Finding 4 — off-spine announcements (§5.2): rejected, as designed.** An
announcement reached conditionally or repeatedly is a compile error. The
alternative would contradict 057's "resolved exactly once" criterion, which
stands. `dev/issues/057` has been amended to enumerate the rejection, since the
design was ahead of the issue here.

**Findings 6 and 7 — seeding `field::value` from an `eq` constraint (§7.2):
accepted.** The observable change to the already-shipped frozen-fields feature
is accepted: a default-constructed `struct_field_list` now reads back its magic
and `eq`-constrained fields as their values rather than zeros. The incidental
repair of the three write-side paths that read zeroed storage (finding 6) is
accepted with it. Finding 7's caveat stands and is now an implementation
obligation rather than an open question: the architect's search for a test
asserting the old zeroed behaviour was by reading, not by building, so the
implementer of 059 confirms it against an actual build before relying on it.
