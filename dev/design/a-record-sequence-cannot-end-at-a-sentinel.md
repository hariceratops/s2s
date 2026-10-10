# Design: a record sequence can end at a sentinel (`until_field_equals<>`)

Spec: `dev/specs/a-record-sequence-cannot-end-at-a-sentinel.md` — frozen. This
design consumes it and does not revise it. Its three Open Questions are settled
in §3.3 (write-side check placement), §8 (naming) and §5.5 (diagnostics). §11
lists where the spec and the issues are incomplete rather than wrong, including
one place where the spec's *mechanization* of its own soundness rule does not do
what the rule's stated purpose requires (F4, F5).

Issues served, in order:
`dev/issues/069-read-a-record-sequence-until-a-sentinel.md` through
`dev/issues/073-document-the-sentinel-terminated-record-form.md`. §7 gives the
slice-by-slice content. 069 is considerably larger than it reads (§11 F1, F2,
F3); 070's first rejection needs no production code at all (§11 F6).

Sits directly on `dev/design/the-size-axis-cannot-say-read-until-a-byte.md`
(062–068), whose `is_delimited_size` is the classification precedent, whose
read-then-test inversion is the loop shape, and whose §1.2 argument — that a
classification trait is a *routing decision*, and that keeping a new size form
out of `is_variable_size` makes five consumer sites correct by construction — is
inherited verbatim rather than restated.

Inherits the governing rule from `dev/design/schema-api-verbosity.md`:

> **a class-type NTTP is a non-deduced context in a partial specialization, so
> anything a trait pattern-matches on must live in the value's *type*, never in
> its value.**

So the sentinel's field id and value both live in the type.

---

## 0. The shape in one page

| Piece | Where | What |
|---|---|---|
| `terminated_by_sentinel_t<id, v>` | `field_size/field_size.hpp` | empty type carrying the named field id and the sentinel value as NTTPs |
| `until_field_equals<id, v>` | `field_size/field_size.hpp` | the spelling; `inline constexpr auto until_field_equals = terminated_by_sentinel_t<id, v>{}` |
| `is_sentinel_terminated_size` / `sentinel_terminated_size_like` | `field_size/field_size.hpp` | the fourth classification trait, beside `is_fixed_size`, `is_variable_size`, `is_delimited_size` |
| `record_sequence_size_like` | `field_size/field_size.hpp` | `variable_size_like \|\| sentinel_terminated_size_like` — the widened gate for `vector_of_records`, and the only one |
| `size_option_like` gains an arm | `field/field_options.hpp` | so `until_field_equals<>` classifies as a size in an option pack at all (§1.4) |
| `is_size_like` gains an arm | `field_size/field_size.hpp` | the "is this on the size axis" predicate |
| `is_vector_of_record_field` gains `requires variable_size_like` | `field/field_traits.hpp` | **narrowed**, so the two record-sequence traits are disjoint (§1.5) |
| `is_sentinel_terminated_record_field` / `sentinel_terminated_record_field_like` | `field/field_traits.hpp` | the field-level trait `read_field` and `write_field` dispatch on |
| `record_sequence_field_like` | `field/field_traits.hpp` | the umbrella — "a run of records, however it ends" — with three users (§1.5) |
| `field_like` gains an arm | `field/field_traits.hpp` | **not optional**; without it `all_field_like` rejects the schema |
| `extract_length_dependencies` arm | `field_list/field_list_metadata.hpp` | **not optional**; the primary is undefined (§1.6) |
| `field_value_of` non-const overload | `field_list/field_list.hpp` | one overload; the write path has to *set* the named field, which `operator[]` cannot reach (§1.7) |
| `matches_sentinel<size>(record)` | `field_list/sentinel_terminator.hpp` (new) | the read test and the write rejection, one function |
| `synthesised_terminator<size, record>()` | `field_list/sentinel_terminator.hpp` | default-construct, set the named field, return |
| `writes_determined_bytes<field>()` | `field_list/sentinel_terminator.hpp` | the recursive field-by-field soundness walk (§5.2) |
| `terminator_field_check<field>` | `field_list/sentinel_terminator.hpp` | one `static_assert` per field, so the offending field is named by the instantiation (§5.5) |
| `terminator_synthesis_check<record, id, v>` | `field_list/sentinel_terminator.hpp` | the named-field obligations (§5.3), shaped like `announcing_record_check` |
| `sentinel_field_exists` / `sentinel_terminator_is_synthesisable` | `field_list/sentinel_terminator.hpp` | the two concepts `vector_of_records` gains |
| `read_field<sentinel_terminated_record_field_like, F>` | `field_read/field_reader.hpp` | the read-then-test loop (§2) |
| `write_field<sentinel_terminated_record_field_like, F>` | `field_write/field_writer.hpp` | per-element rejection, then synthesis (§3) |
| `create_field_from_vector_of_records` widened | `field_read/field_reader.hpp` | constrained on the umbrella |
| two `if constexpr` arms widened | `field_list/announcing_record.hpp` | **silently wrong otherwise** (§1.5, §11 F3) |
| `vector_of_records`'s requires-clause | `api/field_descriptors.hpp` | `record_sequence_size_like` + the two soundness concepts |
| `error_reason::sentinel_not_found` | `error/cast_error.hpp` | appended (069) |
| `error_reason::found_sentinel_in_sequence` | `error/cast_error.hpp` | appended (072) |

Five things deliberately **not** touched, each load-bearing:

- `stream/stream_traits.hpp` — no operation added. The element read is the
  existing `read_field<struct_field_like>`, which is the existing
  `struct_cast_impl`. 069's "no change to `stream_traits.hpp`" is discharged by
  not writing any stream code at all.
- `field_size/field_size_deduce.hpp` and `comptime_field_size_deduce.hpp` — no
  `deduce_field_size` specialization. Its *absence* is the enforcement: a
  sentinel-terminated size resolves to no count, and any path that reached
  `deduce_field_size<until_field_equals<...>>` would stop the build on an
  incomplete type rather than silently read zero elements. Same mechanism the
  companion relies on.
- `field_write/derived_value.hpp` — a sentinel-terminated run obligates nothing
  and is nothing's length source. `len_obligation` requires `variable_size_like`,
  which this size is not, so this falls out of §1.3 rather than needing a rule.
  Note the contrast: a *counted* `vector_of_records` **does** obligate its count
  field today, and keeps doing so.
- `type_deduction/utils/type_tags.hpp` — `as_vector_of_records` keeps
  `variable_size_like`, so a sentinel-terminated run cannot be a union
  alternative. A spec Non-Goal, rejected by an existing clause rather than a new
  one.
- `field_read/read_impl.hpp` and `field_write/write_impl.hpp` — no new primitive.
  The companion had to write `read_delimited` because it read raw bytes; here the
  unit is a record and `struct_cast_impl` already reads one.

---

## 1. The fourth size category, and the trait split it forces

### 1.1 The value and its type

```cpp
// field_size.hpp, beside delimited_by_t

// What a sentinel-terminated record sequence declares: which field of the
// element decides, and the value that ends the run. Both live in the type, for
// the governing reason — read_field tests on them, write_field constructs from
// them, and the option classifier matches on them.
template <fixed_string id, auto v>
struct terminated_by_sentinel_t {
  static constexpr auto sentinel_field = id;
  static constexpr auto sentinel_value = v;
};

template <fixed_string id, auto v>
inline constexpr auto until_field_equals = terminated_by_sentinel_t<id, v>{};
```

`auto v` already carries the spec's "a compile-time constant of any structural
type the named field can hold": structurality is the language's requirement on a
non-type template parameter, not a rule this design imposes, so `u8{0}`,
`fixed_string{"END"}` and `std::array<u8,2>{0,0}` are all admissible spellings
with no arm anywhere. The named field's type still has to accept the value —
that is §5.3's `sentinel_fits_named_field`, checked once and reused by both
directions.

No `sentinel_of<S>` accessor. `size_type_of<S>::sentinel_field` reads it, as
`size_type_of<S>::delim` and `::f` already do for the other two payload-carrying
size types.

### 1.2 The spelling accepts two NTTP kinds, unlike every other size form

`until_field_equals<"size", u8{0}>` is the first size spelling with two template
arguments, and the first mixing a `fixed_string` id with an `auto` value.
Neither is new machinery — `size_from_fields<callable, ids...>` already mixes an
`auto` and a pack of `fixed_string` — but the ordering is worth stating: **id
first, value second**, matching `len_from_field<"size">`'s id-first convention
and reading as an English sentence at the call site.

### 1.3 Why it must not satisfy `variable_size_like`

The companion's §1.2 table applies unchanged; every site keyed on
`is_variable_size` consumes a *count*, and a sentinel-terminated run has none to
give. Restated only where the consequence differs here:

| Site | What `variable_size_like` would have caused |
|---|---|
| `field_size_deduce.hpp` | `field_value_of<field_accessor<...>>` on a `terminated_by_sentinel_t`, which is not a `field_accessor` — a hard error deep in size deduction |
| `field_reader.hpp` (counted vector-of-records) | `resize(n)` on a count that does not exist |
| `field_writer.hpp` (counted vector-of-records) | `deduce_field_size(...) != value.size()` → a spurious `found_contradicting_length` on every write |
| `derived_value.hpp` `len_obligation` | the named field would become a **derived length target of the outer list**, losing its `operator[]` and being overwritten with the run's element count |
| `field_list_metadata.hpp` | the sentinel field id would be recorded as a *sibling* dependency of the outer list, where no such field exists → a constant-evaluation failure in `is_dependencies_resolved` |

The last two are the sharp ones and are specific to this feature: the named field
lives in the *element's* field list, not in the list that declares the run, so
any machinery that treats a size's field name as a sibling name is not merely
useless here, it is wrong. Keeping the trait separate is what makes that
impossible to express.

### 1.4 The option pack

`size_option_like` gains `|| sentinel_terminated_size_like<S>`, exactly as it
gained the delimited arm. Without it `until_field_equals<...>` is an
unrecognised pack entry and `vector_of_records` fails on the per-element
placeholder constraint before any of §1.5's widening is consulted.

The payoff is the same as the companion's, and it is what makes 070's first
rejection free (§11 F6): once the spelling occupies the size slot,
`vector_of_records<"blocks", sub, len_from_field<"n">, until_field_equals<"size", u8{0}>>`
counts as two size options and trips `pack_options`'s existing
`"a field takes at most one size option"` assertion. Two contradictory
statements about what ends the run is, structurally, exactly "two sizes".

The leak the companion had to plug (`basic_field` reaching
`field_fits_to_underlying_type` with an unresolvable size) is already plugged:
064 added `fixed_size_like` ahead of the fits-check, and it rejects this
spelling for the same reason it rejects `until<>`. `str_field` and `vec_field`
take `buffer_size_like`, which this form is deliberately *not* in.
`array_of_records`, `fixed_array_field`, `c_str_field`, `struct_field` and
`variance` take constraint-only packs. So every one of the spec's "not on X"
Non-Goals is enforced by a clause that already exists.

### 1.5 The one existing trait that has to be narrowed, and the umbrella that
follows

`is_vector_of_record_field` matches `field<id, std::vector<T>, size, …>` for
**any** size, with no `requires`. A sentinel-terminated run is a
`field<id, std::vector<T>, …>`, so without a change it satisfies both
`vector_of_record_field_like` and the new trait, and the two `read_field`
partial specializations are ambiguous.

The resolution deliberately does **not** rely on concept subsumption ordering
(defining the new concept as `vector_of_record_field_like<T> && …` so it is more
specialized). Every `read_field`/`write_field` specialization in this codebase is
selected by *disjoint* concepts; none is selected by subsumption, and partial
ordering of class template partial specializations by constraint subsumption is
the corner of C++20 the three supported toolchains agree on least. Instead:

```cpp
// field_traits.hpp — narrowed
template <fixed_string id, field_list_like T, auto size, auto constraint_on_value, auto bound>
  requires variable_size_like<size_type_of<size>>
struct is_vector_of_record_field<field<id, std::vector<T>, size, constraint_on_value, bound>> { … };

// field_traits.hpp — new, disjoint by construction
template <fixed_string id, field_list_like T, auto size, auto constraint_on_value, auto bound>
  requires sentinel_terminated_size_like<size_type_of<size>>
struct is_sentinel_terminated_record_field<field<id, std::vector<T>, size, constraint_on_value, bound>> { … };
```

The narrowing rejects nothing that compiles today: `vector_of_records`'s own
requires-clause and `as_vector_of_records`'s both already demand
`variable_size_like`, so every vector-of-records field in existence has one.

Narrowing an existing trait is not free, and the cost is the reason the umbrella
exists. Three sites ask "is this a run of records?" and mean it regardless of how
the run ends:

```cpp
// field_traits.hpp, immediately after both traits
// A run of records, however it ends. Three sites ask this question and none of
// them cares which form terminates it; two of them answer *silently wrong*
// rather than failing to compile if they are left keyed on the counted trait.
template <typename T>
concept record_sequence_field_like =
  vector_of_record_field_like<T> || sentinel_terminated_record_field_like<T>;
```

| Site | Today | After | What goes wrong otherwise |
|---|---|---|---|
| `field_reader.hpp` `create_field_from_vector_of_records` | `vector_of_record_field_like` | `record_sequence_field_like` | incomplete type — a compile error, found immediately |
| `announcing_record.hpp` `census_of_field` | `vector_of_record_field_like` | `record_sequence_field_like` | **silent**: the `else` arm returns `{0,0}`, so a byte-order-announcing record nested inside a sentinel-terminated run stops being counted and 057's off-spine rejection stops firing |
| `announcing_record.hpp` `is_order_agnostic_field` | `vector_of_record_field_like` | `record_sequence_field_like` | falls through to `is_order_agnostic_value`, which happens to give the right answer via its `vector_like` arm — correct today by accident, and the accident should not be relied on |

`field_like`'s disjunction gains a flat `sentinel_terminated_record_field_like`
arm rather than the umbrella, matching the shape that list already has.

### 1.6 The hard error no issue mentions

`extract_length_dependencies` has no primary definition, deliberately. Its
specializations are constrained on `fixed_size_like`, `size_dont_care_like`,
`variable_size_like && !computed`, `delimited_size_like` and `is_computed_size_v`.
A sentinel-terminated field matches none, and `struct_field_list` instantiates
`field_list_metadata<fields...>` for every schema — so **the first schema
declaring one fails to compile on an incomplete type, before any reader or
writer is reached**, with a diagnostic pointing at metadata extraction.

```cpp
// field_list_metadata.hpp, beside the delimited arm
// The sentinel's field name resolves inside the *element's* field list, not in
// the list that declares the run, so a sentinel-terminated run depends on no
// sibling. The same answer the delimited arm gives, for a different reason, and
// stated for the same reason the size_dont_care arm is.
template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires sentinel_terminated_size_like<size_type_of<size>>
struct extract_length_dependencies<field<id, T, size, constraint, bound>> {
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};
```

### 1.7 Reaching the named field

Both directions need the named field of an element *by id*, and both need to
bypass `operator[]`'s visibility policy — not as a convenience but as a hard
requirement. In the motivating schema `"size"` is `"data"`'s `len_from_field`
target, so it is a length-derived field, and `struct_field_list_impl` gives a
length-derived field **no `operator[]` overload at all**. `terminator["size"_f]`
does not compile, and cannot be made to.

`field_value_of` is already exactly "the lookup `operator[]` performs, without
its visibility policy", and its own comment already justifies the read
direction. The write direction needs one overload:

```cpp
// field_list.hpp, beside the existing const one
template <typename field_accessor, auto list_metadata, typename... fields>
constexpr auto& field_value_of(struct_field_list_impl<list_metadata, fields...>& field_list) {
  constexpr auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id));
  static_assert(field_lookup_res.has_value, "no such field in this field list");
  using field_type_ref = meta::type_of<field_lookup_res->id>&;
  return static_cast<field_type_ref>(field_list).value;
}
```

That is the whole of the mutable access this feature needs, and it is confined
to `synthesised_terminator`. The two helpers built on it:

```cpp
// field_list/sentinel_terminator.hpp

// Cast the sentinel to the named field's type once, rather than promoting the
// field's value up — the same rule read_delimited applies for the same reason:
// a signed char field holding 0xff compared against 0xff promotes to
// -1 == 255 and never matches the very value that should terminate the run.
template <auto size, field_list_like record>
constexpr auto matches_sentinel(const record& r) -> bool {
  using sz = size_type_of<size>;
  const auto& v = field_value_of<field_accessor<sz::sentinel_field>>(r);
  return v == static_cast<std::remove_cvref_t<decltype(v)>>(sz::sentinel_value);
}

template <auto size, field_list_like record>
constexpr auto synthesised_terminator() -> record {
  using sz = size_type_of<size>;
  record r{};
  auto& v = field_value_of<field_accessor<sz::sentinel_field>>(r);
  v = static_cast<std::remove_cvref_t<decltype(v)>>(sz::sentinel_value);
  return r;
}
```

One function serves the read test *and* 072's write rejection, which is what
makes "an element that matches the sentinel is rejected on write" and "an
element that matches the sentinel ends the run on read" provably the same
predicate rather than two that could drift.

### 1.8 The new header, and why it is a header

`field_list/sentinel_terminator.hpp`, modelled on `announcing_record.hpp`: it
sits above `field_list.hpp`, does a compile-time walk over a *nested* field
list, and exposes `*_check` structs whose `res` a concept reads. Both are
things the metadata header cannot do, because reaching a nested record's fields
means pattern-matching `struct_field_list_impl`, which the metadata header is
included *by*.

It is included by `field_read/field_reader.hpp`, `field_write/field_writer.hpp`
and `api/field_descriptors.hpp`. The last is the one to check for a cycle:
`field_descriptors.hpp` already reaches `field_list/field_list.hpp` transitively
through `field_size_deduce.hpp`, so this adds no new edge into `field_list/` —
only a new node above it. `scripts/amalgam.py` topologically sorts the graph
from `include/s2s.hpp`, so a new header needs no registration anywhere.

---

## 2. The read (069)

### 2.1 The loop

```cpp
// field_reader.hpp, after the counted vector-of-records reader
template <sentinel_terminated_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;

  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list) {}

  template <typename stream>
  constexpr auto read(stream& s, cast_endianness& order) const -> rw_result {
    using record = extract_type_from_vec_t<typename T::field_type>;
    using element_field = create_field_from_vector_of_records_v<T>;
    constexpr auto field_size = T::field_size;
    // A compile-time constant, so this is the same "phrase it as a division"
    // trick checked_byte_count uses, and for the same reason: the product
    // n * sizeof(record) is never evaluated on an unvalidated n.
    constexpr auto max_elements = bound_in_bytes<T::field_bound> / sizeof(record);

    field.value.clear();
    while(true) {
      element_field elem;
      auto res = read_field<element_field, F>(elem, field_list).read(s, order);
      if(!res)
        return res;                                      // buffer_exhaustion
      if(matches_sentinel<field_size>(elem.value))
        return {};                                       // consumed, not stored
      if(field.value.size() == max_elements)
        return std::unexpected(error_reason::sentinel_not_found);
      field.value.push_back(std::move(elem.value));
    }
  }
};
```

Three things about this are decisions rather than transcription.

**The element is read through `read_field`, not through a new path.**
`create_field_from_vector_of_records_v<T>` synthesises
`field<T::field_id, record, size_dont_care, no_constraint<record>>`, which is
`struct_field_like`, so `read_field` routes it to `struct_cast_impl<record>`.
That is the identical mechanism `read_buffer_of_records` already uses for the
counted case, reused rather than paralleled. It is also the whole of 069's "no
change to `stream_traits.hpp`": there is no stream code in this feature.

**`read_buffer_of_records` itself is *not* reused.** It takes a count, loops to
it, and assigns by index into an already-resized vector — all three of which are
what this feature inverts. Widening it to do both would mean a `len_to_read` that
is sometimes meaningless and a branch at every one of its three lines.

**`field.value.clear()` is not defensive.** It states that the reader replaces
the container rather than appending, which makes it locally correct instead of
correct only for the callers that happen to pass a fresh field. Same reason
`read_delimited` opens with it.

### 2.2 Where the bound is enforced, and why it is a comparison rather than
`checked_byte_count`

The denominator is `n * sizeof(record)` — the **in-memory** footprint, not wire
bytes. That is the spec's choice and it is already the denominator the counted
`vector_of_records` reader uses, for the reason its own comment gives: a record
carrying variable-length subfields has no statically known wire size. This
differs from the companion, whose bound counts the value's bytes because a
delimited field's bytes *are* its value.

`checked_byte_count` is deliberately not called, on the companion's §2.2
reasoning, which transfers exactly:

- It exists to validate a length that came *off the wire* before allocating by
  it. There is no such length here; the container grows one element at a time
  under a loop that cannot pass the ceiling, so the product it guards cannot
  occur.
- It reports `excessive_length`, documented in `docs/errors.md` as "a declared
  length was never satisfiable, and nothing was allocated". Reaching the bound
  here means the opposite: everything up to the bound *was* read and allocated
  and no sentinel was among it. That is `sentinel_not_found`.

Note that the *counted* reader keeps its `checked_byte_count` call — it has a
wire-supplied count and is exactly the case the function exists for. The two
coexist because they are answering different questions.

**Placement: after the sentinel test, before the `push_back`.** 069's criterion
says the bound is checked "after every `push_back`, not once up front"; the
contrast it is drawing is per-element versus up-front, and both placements
satisfy it with identical accept/reject behaviour. Checking before the push is
strictly better on the one axis where they differ: checking after would grow the
vector to `max_elements + 1` records — allocating and *moving* one more record
than the bound permits — before rejecting. For a record holding vectors that is
not a rounding error. This is also where the companion put its check, for a
related but distinct reason (its off-by-one was about the delimiter byte; there
is no analogous byte here, because the sentinel element is tested before the
bound is consulted and so is never counted against it).

### 2.3 The three outcomes

| Outcome | Reported as | Raised by |
|---|---|---|
| sentinel element read | success; element consumed, not stored | the loop |
| stream ran dry first | `buffer_exhaustion` | `read_native_impl`, through `struct_cast_impl`, unchanged |
| bound reached first | `sentinel_not_found` (new) | the loop |

A terminator in first position returns success with an empty vector, having
consumed the terminator's bytes. Not an error: a minimum length is a constraint
the schema author writes.

`buffer_exhaustion` arrives through the nested `struct_cast_impl`'s narrowing
seam with no new code, which is the right outcome — "the stream ran dry" is the
same fact here as everywhere else. Note the consequence: a stream that ends
*mid-element* and one that ends *between* elements both report
`buffer_exhaustion`, because the element read is atomic from this loop's point of
view. That is correct and worth documenting: both are truncation.

**The bound is load-bearing in a second way the spec does not name.** A schema
whose element can occupy zero wire bytes — every field an empty container behind
a `maybe` — would make this loop spin without advancing the stream. It
terminates at `sentinel_not_found` rather than hanging. (Such an element is
rejected at compile time by §5's soundness check anyway, but the loop does not
depend on that being true.)

---

## 3. The write (071, 072)

### 3.1 The specialization

```cpp
// field_writer.hpp, after the counted vector-of-records writer
template <sentinel_terminated_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <typename stream>
  constexpr auto write(stream& s, cast_endianness& order) const -> rw_result {
    using record = extract_type_from_vec_t<typename T::field_type>;
    constexpr auto field_size = T::field_size;

    for(const auto& rec: value) {
      // Before this element's bytes leave, not before the run's: §3.3.
      if(matches_sentinel<field_size>(rec))
        return std::unexpected(error_reason::found_sentinel_in_sequence);
      auto res = write_nested<record>(s, rec, order);
      if(!res)
        return res;
    }
    return write_nested<record>(s, synthesised_terminator<field_size, record>(), order);
  }
};
```

An empty vector emits the lone synthesised terminator — the loop simply does not
run. No special case, and none of the companion's F7 hazard
(`const_byte_addressof` on an empty container), because a record is written
field by field rather than as a block.

Neither `is_derived_target_v` nor `is_frozen_target_v` is consulted. A
sentinel-terminated field is neither: it cannot be frozen
(`frozen_field_like` requires `fixed_sized_field_like`) and it cannot be derived
(§1.3). No branch says so; the traits do not match.

`failed_at` naming the field is free — `stream_cast_impl`'s fold attaches
`fields::field_id` to every `rw_result` a writer returns, so 072's criterion
needs a test and no code. The field it names is the sequence field (`"blocks"`),
not a field of the offending element; that is the existing documented behaviour
of every nested write and is the right answer here, since the offending thing is
the run's contents.

### 3.2 The synthesis, and the one thing that can defeat it at run time

`synthesised_terminator` default-constructs the record and sets the named field.
Everything else follows from what default construction leaves: every
`std::vector`/`std::string` is empty, every scalar is zero or its `eq` seed. So
a `len_from_field` container in the terminator is empty, and — crucially — the
write path *derives* that container's length slot from the container, writing
zero into it.

That is exactly why the GIF terminator is one zero byte, and it is also the
mechanism §5.3 has to guard: if the **named** field is itself such a length slot,
the derived zero overwrites the sentinel the synthesis just stored. With a zero
sentinel the two agree and the terminator is correct; with a non-zero sentinel
the wire gets a byte the reader will never accept as a terminator, and the run
never ends. §5.3 rejects that schema at compile time.

### 3.3 Where the sentinel check fires — the spec's first Open Question, settled

**Decision: per element, immediately before that element is written. No pre-pass
over the vector, and no rollback.** This confirms the PRD's default, and 072's
acceptance criteria, with a reason rather than by deferral.

The pre-pass is genuinely available — the whole vector is in hand — and it would
buy the property the companion's delimited writer does have: on rejection, the
stream is byte-identical to what it was before the field. Three reasons it is
still the wrong choice here, the third being decisive:

1. **It contradicts a frozen acceptance criterion.** 072 states in as many
   words: "failure happens at the offending element with no rollback — earlier
   elements may already be on the stream… The whole vector is not pre-validated
   before anything is emitted." The spec leaves the question open; the issue does
   not.
2. **It costs a second traversal of the whole vector on every successful
   write** — the overwhelmingly common case — to improve only the failing one.
   The companion's scan was over the same bytes about to be written as one block;
   this is a second walk over records.
3. **The guarantee it appears to buy does not exist for this construct
   anyway.** Each element is written by a nested `stream_cast_impl`, which has no
   rollback of its own: a record whose third field fails its constraint leaves
   the first two fields' bytes on the stream. So "this field leaves the stream
   untouched on failure" is *already* false for a vector of records under every
   other error. A pre-pass would establish it for exactly one error class and
   nowhere else — a guarantee that is worse than not having it, because a caller
   could believe it holds generally.

The rejection is therefore at element granularity, and the property that *is*
guaranteed is stated plainly in the docs: the offending element's own bytes never
reach the stream.

**There is no "already at the end, treat it as the terminator" exemption**, per
072. A match in the last position is rejected like any other. This falls out of
the scan being unconditional — there is no trailing-element special case to
write, and writing one would be the bug: the vector holds *real* elements by
069's definition, so a caller who put the terminator in it has a different bug
that this reports.

**A note on the compile-time tier.** Under `ut`'s compile-time mode the whole
write is a constant expression, so this check executes during constant
evaluation and produces `std::unexpected(found_sentinel_in_sequence)` at compile
time, asserted on as a value. Nothing is deferred and nothing is a diagnostic —
the companion's §3.2 reasoning about not turning a runtime rejection into a hard
error applies verbatim and is not re-argued here.

---

## 4. Naming — the spec's second Open Question, settled

**Decision: keep `until_field_equals<"size", u8{0}>`.**

It reads well beside `until<u8{0}>` for a specific reason rather than by
resemblance: both begin `until`, so the size axis's two discovered-length forms
announce themselves as a family, and the suffix carries the entire difference —
one stops at a *byte* equal to a value, the other at a *field of the element*
equal to a value. A reader who knows one can predict the other's semantics from
its name, which is the test a family of spellings has to pass.

Alternatives considered and dropped:

- **`until_record_where<"size", eq{0}>`** — borrows the constraint DSL, and so
  advertises a general predicate. The spec rejects predicates as terminators
  precisely because they cannot be run backwards to produce what they accept, and
  a spelling that invites `until_record_where<"size", lt{4}>` invites the thing
  the feature cannot do.
- **`terminated_by<"size", u8{0}>`** — leaves the `until` family and reads as
  though the *field* did the terminating. Kept as the internal type name
  (`terminated_by_sentinel_t`), where it is accurate and where nobody types it.
- **`until_field_is<…>`** — "is" is vaguer than "equals" and has the same
  predicate-inviting problem in miniature.
- **`until_element<"size", u8{0}>`** — "element" is the user's word for a vector
  member and is fine, but it drops the word that says *what kind of test*, which
  is the one thing the name has to carry once a second termination form exists.

The cost is length. Accepted: it is written once per schema, and the length is
carrying the rule. Per the spec's own direction, the standing naming sweep
(`dev/inbox/todos.md`) is last and will see this and `until<>` together; nothing
here should block on it.

---

## 5. The synthesis-soundness check (070)

### 5.1 What the check actually has to establish

The spec states the rule as:

> Every field of the element other than the named one must have a determined
> width in the synthesised terminator: each is either fixed-width, or has its
> length driven — directly or transitively — by the named field.

The *purpose* clause ("must have a determined width in the synthesised
terminator") and the *mechanization* clause ("fixed-width, or driven by the named
field") do not pick out the same set of schemas, and the mechanization is wrong
in both directions. This design implements the purpose. §11 F4 and F5 report the
divergence as findings rather than burying it here.

Briefly, why the mechanization does not hold. Synthesis default-constructs the
element, so **every** `len_from_field` container in the terminator is empty and
emits zero bytes — whether or not the field driving it is the named one. So
"driven by the named field" is not what makes such a field's width determined;
being a length-prefixed container is. Conversely, what "driven by the named
field" genuinely controls is something the spec's rule does not mention at all:
whether the named field's *stored* sentinel survives the write path's length
derivation (§3.2). That is the real silent-corruption case, and the spec's rule
admits it.

So the check has two halves, and they are about different things.

### 5.2 Half one: every field of the element emits determined bytes

Expressed as a recursive `constexpr` function over field kinds with a
`static_assert(dependent_false<T>)` in the `else`, which is exactly
`is_order_agnostic_field`'s shape in `announcing_record.hpp` — chosen so that a
field kind added later is a hard error here rather than a silent answer.

```cpp
// field_list/sentinel_terminator.hpp
template <typename T>
constexpr auto writes_determined_bytes() -> bool;

template <typename L>
struct list_writes_determined_bytes;

template <auto metadata, typename... fields>
struct list_writes_determined_bytes<struct_field_list_impl<metadata, fields...>> {
  static constexpr bool res = (writes_determined_bytes<fields>() && ...);
};

template <typename T>
constexpr auto writes_determined_bytes() -> bool {
  if constexpr(fixed_sized_field_like<T>)
    return true;                  // its declared byte_count, whatever the value
  else if constexpr(delimited_field_like<T>)
    return true;                  // an empty value plus its delimiter: one byte
  else if constexpr(variable_sized_field_like<T>)
    // The container is empty in a default-constructed element, so it emits
    // nothing and its length slot derives to zero. A computed size is the
    // exception: it is a callable over values this check does not have, and on
    // write it is *verified* against the container rather than derived from it,
    // so a callable returning non-zero for the default element fails the
    // terminator's write with found_contradicting_length.
    return !is_computed_size_v<size_type_of<T::field_size>>;
  else if constexpr(struct_field_like<T>)
    return list_writes_determined_bytes<extract_type_from_field_v<T>>::res;
  else if constexpr(array_of_record_field_like<T>)
    return list_writes_determined_bytes<extract_type_from_array_v<typename T::field_type>>::res;
  else if constexpr(record_sequence_field_like<T>)
    // A counted run is empty, so no elements are written. A nested
    // sentinel-terminated run emits its own synthesised terminator, whose
    // soundness its own declaration already had to establish.
    return true;
  else if constexpr(optional_field_like<T> || union_field_like<T> ||
                    order_announcing_field_like<T>)
    // Each needs a value this check does not have: a presence predicate over
    // siblings, a variant's held index against a discriminant, a byte order
    // resolved from bytes. None can be shown determined from types alone.
    return false;
  else
    static_assert(dependent_false<T>,
      "whether this field kind writes determined bytes in a synthesised "
      "sentinel terminator is unknown to the soundness check");
}
```

`order_announcing_field_like` returning `false` is belt-and-braces — 057's
off-spine census already rejects an announcement inside any record sequence — and
is listed rather than left to the `else` so that the `else` keeps meaning "a kind
nobody has considered".

### 5.3 Half two: the named field's obligations

Shaped like `announcing_record_check` and `dependency_check` — a struct of
`static_assert`s whose `res` a concept reads, so a bad schema fails as a sentence
*and* as an unsatisfied constraint:

```cpp
template <typename record, fixed_string sentinel_field, auto sentinel>
struct terminator_synthesis_check;

template <auto metadata, typename... fields, fixed_string S, auto v>
struct terminator_synthesis_check<struct_field_list_impl<metadata, fields...>, S, v> {
  using named = meta::type_of<lookup_field<metadata>(as_sv(S))->id>;
  using named_type = typename named::field_type;

  // 1. fixed width. The sentinel has to be settable and comparable, and the
  //    terminator's own width has to be determined like every other field's.
  //    Admits arrays and fixed_strings, so the spec's "any structural type the
  //    named field can hold" is honoured — it is not an integral-only rule.
  static constexpr bool named_is_fixed_width = fixed_sized_field_like<named>;
  static_assert(named_is_fixed_width, "…");

  // 2. the sentinel is a value the named field can hold, and can be compared
  //    against it. One requirement, two users: the read test casts it down, the
  //    synthesis assigns it.
  static constexpr bool sentinel_fits = sentinel_fits_named_field<named_type, v>;
  static_assert(sentinel_fits, "…");

  // 3. the named field is not pinned by eq. If it were, every element of the
  //    run would carry the pinned value, so either no element could ever be a
  //    non-terminator (when eq's value is the sentinel) or the terminator could
  //    never be written (when it is not).
  static constexpr bool named_not_frozen = !frozen_field_like<named>;
  static_assert(named_not_frozen, "…");

  // 4. the terminator satisfies the named field's own constraint. Evaluated,
  //    not inferred: constraint objects are constexpr callables, so this is a
  //    compile-time call. Without it a `gt{0}` on the named field turns every
  //    terminator write into a runtime validation_failure.
  static constexpr bool sentinel_satisfies_constraint =
    named::constraint_checker(static_cast<named_type>(v));
  static_assert(sentinel_satisfies_constraint, "…");

  // 5. §3.2's real corruption case. If the named field is another field's
  //    length target, the write path derives its value from that field's
  //    contents and discards the stored one. In the terminator that container
  //    is empty, so the byte written is zero.
  static constexpr bool sentinel_survives_derivation =
    !is_length_derived_field<metadata>(as_sv(S)) ||
    (static_cast<named_type>(v) == named_type{});
  static_assert(sentinel_survives_derivation,
    "the named field is the declared length of another field in this element, "
    "so the write path derives its value from that field's contents rather than "
    "from the stored one. The synthesised terminator's containers are empty, so "
    "the value written is zero — a non-zero sentinel would never reach the wire "
    "and a run written this way would never terminate on read");

  static constexpr bool fields_determined =
    (terminator_field_check<fields>::res && ...);

  static constexpr bool res = named_is_fixed_width && sentinel_fits &&
                              named_not_frozen && sentinel_satisfies_constraint &&
                              sentinel_survives_derivation && fields_determined;
};
```

Two notes on reuse. `is_length_derived_field<metadata>` is the metadata table
`operator[]`'s own visibility policy consults — the same single source of truth,
so the check cannot drift from what the write path actually does. And the
discriminant case needs no arm: if the named field were a union's discriminant,
that union field is already rejected by §5.2.

The named field is checked by §5.2 as well as here; no exclusion is needed,
because obligation 1 is strictly stronger than what §5.2 would ask of it.

### 5.4 Where it attaches

Two concepts, on the companion's `delimited_buffer_is_byte_wide` pattern — an
implication over the pair, so a `vector_of_records` declaring a count never sees
either rule:

```cpp
// The record's metadata, recovered by pattern-matching struct_field_list_impl.
// announcing_record.hpp's field_table_of is the same six lines for the same
// reason; reusing it would make this header depend on the announcement
// machinery, which has nothing to do with sentinels. Repeat the pattern-match,
// not the table.
template <typename T>
struct metadata_of;

template <auto metadata, typename... fields>
struct metadata_of<struct_field_list_impl<metadata, fields...>> {
  static constexpr auto value = metadata;
};

template <typename S, typename record>
concept sentinel_field_exists =
  !sentinel_terminated_size_like<S> ||
  lookup_field<metadata_of<record>::value>(as_sv(S::sentinel_field)).has_value;

template <typename S, typename record>
concept sentinel_terminator_is_synthesisable =
  !sentinel_terminated_size_like<S> ||
  terminator_synthesis_check<record, S::sentinel_field, S::sentinel_value>::res;
```

Both attach to `vector_of_records`'s requires-clause, **conjoined in that
order**. The ordering is load-bearing: `terminator_synthesis_check` opens with
`meta::type_of<lookup_field<…>(…)->id>`, which is ill-formed when the lookup
found nothing, and conjunction short-circuits during constraint checking — the
same two-stage shape `basic_field` uses to keep `field_fits_to_underlying_type`
from being substituted for a non-fixed size.

The descriptor, not `struct_field_list`, is the attachment point: the descriptor
is where the (element, sentinel field, sentinel value) triple is spelled, so it is
where the diagnostic can point at what the author wrote. A hand-rolled
`field<…>` bypasses it, as it bypasses every other descriptor-level rule in this
codebase.

### 5.5 Naming the offending field — the spec's third Open Question, settled

**Decision: the `static_assert` message cannot name the field, and does not try.
The field is named by the *instantiation*, via a one-field check struct.**

The message cannot carry it because a C++23 `static_assert` takes a string
literal; user-generated messages are P2741, which is C++26, and this project's
baseline is C++23 with gcc 14 as the constexpr floor. There is no spelling that
interpolates a `fixed_string` into the text.

But the name does not have to be in the text. Making the per-field predicate a
per-field *entity* puts the field in the failing instantiation's template
argument list, which every supported compiler prints as instantiation context:

```cpp
template <typename f>
struct terminator_field_check {
  static constexpr bool res = writes_determined_bytes<f>();
  static_assert(res,
    "this field of the terminating element does not write a determined number "
    "of bytes when the element is default-constructed, so the synthesised "
    "terminator would not be the minimal sentinel the format expects. Every "
    "field of the element must be fixed-width, a length-prefixed container "
    "(which is empty in the terminator), a delimited field, or a nested record "
    "of such — not a computed size, an optional, or a union");
};
```

`(terminator_field_check<fields>::res && ...)` then reports the sentence *and*
`in instantiation of 'struct s2s::terminator_field_check<s2s::field<{"crc"}, …>>'`,
whose first template argument is the offending field's id.

This is also why the check is not written as a fold in a requires-clause.
`field_options.hpp` states the reason for the equivalent choice there: "a fold
names the fold and dumps the whole pack, while this isolates the offending
entry". The same applies, and here the isolation is the entire answer to the
question.

Registration follows 070's instruction and this directory's policy: the two
`must_not_compile` cases use `add_rejected_case`, not
`add_rejected_case_matching`, and pin no text. The named-field obligations in
§5.3 carry sentences because they are `static_assert`s and a sentence is free
there; nothing depends on those sentences being matched. Each case must be
checked against a no-`CASE` control build and confirmed to fail on its own
diagnostic before registration — the trap 047 fell into.

---

## 6. What the terminating element may and may not contain, stated for the docs

Falls out of §5.2, collected here because it is the rule a schema author needs:

| In the element | Terminator emits | Allowed |
|---|---|---|
| fixed-width field (`basic_field`, `fixed_array_field`, `magic_*`, `c_str_field`, `fixed_string_field`) | its declared width, value-initialized or `eq`-seeded | yes |
| `len_from_field` container (`str_field`, `vec_field`, counted `vector_of_records`) | nothing; its length slot derives to zero | yes |
| `until<d>` field | one delimiter byte | yes |
| nested `struct_field` / `array_of_records` | the sum of its fields, recursively checked | yes, if the nested record passes |
| nested `until_field_equals` run | its own synthesised terminator | yes |
| `size_from_fields` computed size | unknown — the callable is not evaluable here | **no** |
| `maybe<…>` | unknown — the presence predicate reads sibling values | **no** |
| `variance<…>` | unknown — the held alternative must agree with a discriminant | **no** |
| the named field, pinned by `eq` | the pinned value, never the sentinel | **no** |
| the named field, when it is another field's `len_from_field` target, with a non-zero sentinel | zero | **no** |

The GIF sub-block and DNS label witnesses both land in rows 1, 2 and 10-with-a-
zero-sentinel, which is why they pass. They must be asserted as a positive
control in `test/schema/`, per 070.

---

## 7. Slice by slice

**069 — the read.** `terminated_by_sentinel_t`, `until_field_equals`,
`is_sentinel_terminated_size`/`sentinel_terminated_size_like`,
`record_sequence_size_like`, the `size_option_like` arm, the `is_size_like` arm,
`is_vector_of_record_field`'s narrowing, `is_sentinel_terminated_record_field`/
`_like`, `record_sequence_field_like` and its three users, the `field_like` arm,
the `extract_length_dependencies` arm, `field_value_of`'s non-const overload, the
new header with `matches_sentinel`, the `read_field` specialization,
`vector_of_records`'s widened size clause, `error_reason::sentinel_not_found`.
Materially larger than the issue reads — §11 F1, F2, F3.

**070 — the two rejections.** `writes_determined_bytes`,
`list_writes_determined_bytes`, `terminator_field_check`,
`terminator_synthesis_check`, `sentinel_fits_named_field`, the two concepts, and
the two conjuncts on `vector_of_records`. The contradictory-declaration
rejection needs **no production code** (§11 F6) — one `must_not_compile` entry
against the existing duplicate-size assertion. Plus the positive-control
assertions on both witnesses in `test/schema/`.

**071 — the synthesis.** `synthesised_terminator` and the `write_field`
specialization *minus* the per-element scan, i.e. §3.1 with the two scan lines
removed. Must land with 072 in one reviewable unit: between them the branch
writes vectors that read back short with nothing raising an error, which is the
hazard the feature exists to prevent. Same coupling the companion's 065/066 had.

**072 — the rejection.** The two scan lines, and
`error_reason::found_sentinel_in_sequence`.

**073 — documentation.** §10.

---

## 8. Decisions, stated explicitly

### State and data lifecycle

**Read: the vector is the unit, and partial completion is never observable.**
The loop clears and grows the caller's container; on failure it returns with the
container holding whatever it had accumulated, and `struct_cast_impl` discards
the whole `struct_field_list` on the first failure — so a partially filled vector
never reaches a caller. Building into a temporary and swapping on success would
allocate twice for a property nobody can observe.

**Read: the stream position after a failure is not restored, and cannot be.**
The stream concept has no seek, deliberately. After `sentinel_not_found` the
position is `max_elements + 1` elements past where the field started; after
`buffer_exhaustion` it is wherever the failing element left it, possibly
mid-record. This is already true of every failing read in the library and is why
failure is terminal for the whole cast rather than recoverable per field.

**Write: the unit is one element, and this is the feature's one genuine
partial-completion decision.** §3.3. The offending element's own bytes never
reach the stream; earlier elements' bytes do. The property deliberately *not*
claimed is "the field leaves the stream untouched" — that is false for a vector
of records under every other error too, and a pre-pass that made it true for this
one error class would be a guarantee a caller could over-generalize.

**Write: the synthesised terminator is constructed fresh per write and owned by
the writer's frame.** It is a value returned by `synthesised_terminator`,
never a cached or shared object, so nothing about it survives the call. This
matters for the constexpr path, where a cached static would have to be a
`constexpr` object and a record containing `std::vector` cannot be one.

Nothing here is reversible, replayable or auditable, and nothing needs to be: a
cast is a pure function from a stream to a value, and a write is a pure function
from a value to bytes.

### Error propagation

Unchanged in structure; two new leaves, both **appended** so existing enumerator
values stay stable, per the precedent `found_contradicting_length`,
`excessive_length`, `delimiter_not_found` and `found_delimiter_in_value` set.

The layering is untouched. `read_field`/`write_field` return `rw_result`
carrying a bare `error_reason` and knowing nothing about field names;
`struct_cast_impl`/`stream_cast_impl` attach `failed_at` from `fields::field_id`.
That is the one seam where the representation changes, and this feature adds no
second one — but it does lean on an existing second narrowing that is worth
naming: `write_nested` and `read_field<struct_field_like>` both take a nested
`cast_error` and drop its `failed_at`, so a failure *inside* an element is
reported against the sequence field. That is the documented behaviour for every
nested record and is the right answer here (the offending thing is the run), but
it means a caller cannot learn which element failed from the error alone.

- `sentinel_not_found` — the bound was reached with no sentinel element. New
  because `buffer_exhaustion` would be diagnostically useless: truncated and
  corrupt are different facts about a file and a caller can act differently on
  them.
- `found_sentinel_in_sequence` — the write-side rejection. Deliberately not
  folded into `validation_failure`: the author has not violated a constraint they
  wrote, they have hit a rule of the termination form. Same argument, same shape,
  as `found_delimiter_in_value`.

The layer below is reused rather than wrapped: `buffer_exhaustion` reaches the
caller from the element read untouched, and so does any error the element's own
fields raise (`excessive_length` from a sub-field's bound, `validation_failure`
from a sub-field's constraint).

### Concurrency and ownership

**Single-threaded, no shared mutable state, and no change on this axis.** Stated
rather than assumed, because one thing in this feature *looks* like shared state
and is not.

Every object is owned by one call chain: the `struct_field_list` belongs to
`struct_cast_impl`'s frame, the vector belongs to the field, each element is
constructed in the loop body and moved in, the stream belongs to the caller and
is borrowed by reference for the duration of the cast. The `cast_endianness&`
cell threaded through the readers is the one piece of state outliving a single
field — this feature passes it through to the element read and never writes it,
and §1.5 keeps the announcing-record census able to see into a sentinel run so
that a schema which *would* write it stays rejected.

`synthesised_terminator` returns by value into the writer's frame and is
destroyed at the end of the full expression. It is deliberately not a
function-local `static` or a namespace-scope `constexpr`, which would be shared
mutable state under the first schema whose element holds a container.

Two concurrent casts over one stream were never supportable and still are not.

### Reuse

Used rather than reimplemented:

- **`read_field<struct_field_like>` / `struct_cast_impl`** as the element reader,
  and `write_nested` as the element writer. This is the decision that makes the
  feature contain no stream code at all, and it is what discharges 069's
  "no change to `stream_traits.hpp`".
- **`create_field_from_vector_of_records_v`** — the same synthesised element
  field the counted reader uses, with one widened constraint rather than a
  parallel metafunction.
- **`field_value_of`** — already "the lookup `operator[]` performs, without its
  visibility policy", which is precisely the capability the named field needs
  (§1.7). One overload added; no new lookup path.
- **`is_length_derived_field`** — the same table `operator[]`'s visibility policy
  reads, so §5.3's obligation 5 cannot drift from what the write path does.
- **`max_bytes` / `bound_in_bytes` / `field_bound`** — the whole bound vocabulary
  from 046–047, repurposed a second time, exactly as the companion repurposed it.
- **`is_order_agnostic_field`'s shape** for §5.2's recursive walk, and
  **`announcing_record_check`'s shape** for §5.3's check struct — both are the
  project's established answers to "walk a nested field list at compile time" and
  "fail as a sentence and as an unsatisfied constraint".
- **`checked_byte_count`'s phrasing-as-a-division**, as a form rather than a
  call (§2.2).

Introduced here and worth factoring for the next comparable piece of work:

- **`record_sequence_field_like`** is the umbrella the *next* termination form
  for record runs will also want. Adding one means one new narrow trait and one
  more arm in the umbrella, with no site-by-site audit — which is exactly the
  audit §1.5 had to do this time because the umbrella did not exist.
- **`matches_sentinel` / `synthesised_terminator`** are free functions over
  `(size, record)` rather than members of a reader or writer, so the read test
  and the write rejection are provably one predicate, and so a future
  region-bounded or nested variant can call them without inheriting a field
  reader.
- **`writes_determined_bytes`** answers a question broader than this feature:
  "what does this field emit in a default-constructed record?" Any later feature
  that has to synthesise a record — a padding form, a default-record form — needs
  the same walk.

Deliberately *not* reused: `read_buffer_of_records` (§2.1), `checked_byte_count`
as a call (§2.2), and concept subsumption for dispatch ordering (§1.5).

### Extension points

Three, each with the contract that holds it stable:

- **A second element-terminator form** — a multi-field terminator, or a
  terminator over a nested field's value. The contract is
  `record_sequence_field_like` plus the fact that `read_field` and `write_field`
  dispatch on disjoint field traits: a new form adds a size type, a narrow field
  trait, an umbrella arm and one specialization per direction, and touches
  nothing that exists. `until_field_equals`'s own name reserves the shape by
  naming what it tests.
- **Region-bounded termination**, deferred by the spec for the same reason the
  companion deferred it: the read path carries no notion of remaining-region
  bytes. The contract is that the bound is read from `T::field_bound` inside the
  loop and nowhere else, so threading a budget later changes one comparison, not
  every reader's signature. The cost that keeps it out is exactly that threading.
- **`maybe` wrapping a sentinel-terminated run** — already works and is in no
  slice's scope (§11 F9). `to_optional_field_v` rewrites the field type to
  `std::optional<std::vector<T>>`, which matches `is_optional_field` and neither
  record-sequence trait, and `read_field<optional_field_like>` delegates to the
  base field. Recorded because a user finds this kind of thing before a
  maintainer does.

Explicitly **not** extension points, each a spec Non-Goal rejected today by an
existing clause rather than a new one: runtime-valued sentinels (no spelling
exists and the size type has no slot for one), stream lookahead
(`stream_traits.hpp` untouched), `until_field_equals` on `array_of_records`,
`vec_field`, `str_field`, optionals or unions (constraint-only packs,
`buffer_size_like`, and `as_vector_of_records`'s `variable_size_like`
respectively), and multi-condition terminators (the type carries one id and one
value, and the spelling does not generalize by accident).

### Build vs. buy

No dependency is possible or wanted: this is a header-only library amalgamated
into a shipped single header, and every component is a trait, a loop, or a
compile-time walk.

The genuine build-vs-buy question here is not about a library, it is **whether
to build a new read primitive at all** — and the answer, unusually, is no. The
companion had to build `read_delimited` because its unit was a raw byte and
nothing existed that read bytes until one matched. This feature's unit is a
record, and `struct_cast_impl` already reads exactly one record from a stream and
reports the three outcomes correctly. So the element read is bought from inside
the project, and what is built is only the loop around it. The evaluation that
matters:

- **`read_buffer_of_records`** was evaluated as the loop to extend and rejected
  (§2.1): its three lines are a count, a bound-to-that-count loop, and an
  indexed assignment, and all three are what this feature inverts.
- **`read_delimited`** was evaluated as a model and correctly not extended: it
  reads `T::value_type` per iteration through `read_native_impl`, which is the
  byte-level primitive. A record is not a value type.
- **Stream lookahead** (`peek`/`unget`) was evaluated by the spec and rejected on
  merits, and this design adds one datum in its favour being *unnecessary*: both
  witnesses' terminators are self-contained degenerate elements, so read-then-test
  consumes exactly the bytes the terminator occupies and strands nothing. There
  is no case in the spec's format scan where the unconsumed-terminator property
  would be observable.

For the soundness check the same question arises and lands the same way: the walk
is built, but its two halves reuse `is_order_agnostic_field`'s recursion shape and
`field_list_metadata`'s existing `length_derived_field_ids` table rather than
re-deriving either.

### Abstractions introduced

Each with the specific problem that forces it. Anything without one has been
collapsed.

| Introduced | Forcing problem |
|---|---|
| `terminated_by_sentinel_t<id, v>` | a trait must pattern-match the field id and the value, and a class-type NTTP is a non-deduced context in a partial specialization |
| `is_sentinel_terminated_size` / `sentinel_terminated_size_like` | routing: a fourth size category needs telling apart from the three that resolve a count or a delimiter before the read |
| `record_sequence_size_like` | one descriptor must admit exactly one more thing, and `variable_size_like` cannot be the place to say it (§1.3) |
| `is_sentinel_terminated_record_field` / `_like` | `read_field` and `write_field` dispatch on field traits, not size traits, and the two specializations must be selected by disjoint concepts rather than by subsumption |
| `record_sequence_field_like` | three sites ask "is this a run of records?", and two of them answer silently wrong if left keyed on the narrowed trait (§1.5) |
| `sentinel_fits_named_field` | the read comparison and the write assignment need the same convertibility, and stating it twice is how they drift |
| `matches_sentinel` | the read test and the write rejection must be one predicate, and each has a different caller |
| `synthesised_terminator` | the terminator is constructed in one direction and from two slices (071 writes it, 070 proves it sound) |
| `writes_determined_bytes` | the soundness rule is recursive over field kinds and over nested records; an inline fold cannot recurse |
| `terminator_field_check` | naming the offending field requires it to be a template argument of the failing entity (§5.5) |
| `terminator_synthesis_check` | five named-field obligations that must fail as sentences *and* feed a concept, following `announcing_record_check` |
| `field_value_of`'s non-const overload | the synthesis must *set* a field that `operator[]` deliberately does not expose (§1.7) |
| `error_reason::sentinel_not_found` | truncated and corrupt are different facts about a file |
| `error_reason::found_sentinel_in_sequence` | a termination-form rule is not a constraint the author wrote, and `validation_failure` would say it was |

Collapsed rather than introduced, each having failed the same bar:

- **`sentinel_of<S>` / `sentinel_field_of<S>`** — `size_type_of<S>::sentinel_field`
  already reads it, as `::delim` and `::f` do for the other two.
- **A `terminator_options` pack kind** beside `bound_pack_option_like` —
  `until_field_equals<>` *is* a size, so `size_in_pack` and the duplicate-size
  assertion serve it unchanged, and that is what makes 070's first rejection free.
- **A `deduce_field_size` specialization** returning zero or an optional — its
  absence is the enforcement (§0).
- **A `sentinel_run_reader` class** parallel to `read_buffer_of_records` — the
  loop has one caller and one instantiation shape; a class would be a constructor
  and three members wrapping eleven lines.
- **A general "record synthesis" facility** taking a field/value map — one field
  is set, by one caller, in one direction.
- **A pre-pass validator over the vector** (§3.3).

### Alternatives rejected

**Widen `is_variable_size` to include the sentinel form and special-case the
consumers.** The smallest diff, and it inverts the dependency: five sites would
need "unless it is sentinel-terminated" branches, and two of them
(`derived_value.hpp`, `field_list_metadata.hpp`) would be interpreting a field id
that names a field of the *element* as if it named a sibling. Rejected for the
companion's reason, which is sharper here: the five correct behaviours are free
if the trait does not match, and actively wrong if it does.

**Distinguish the two `read_field` specializations by concept subsumption** —
define `sentinel_terminated_record_field_like` as
`vector_of_record_field_like<T> && …` and let partial ordering pick the more
constrained one. Rejected in §1.5: no `read_field` specialization in this
codebase is selected that way, and constraint-based partial ordering of class
template partial specializations is the corner of C++20 the three supported
toolchains agree on least. Narrowing the existing trait costs one `requires`
clause and rejects nothing that compiles today.

**Leave `is_vector_of_record_field` unnarrowed and dispatch inside one
`read_field` with `if constexpr`.** Rejected: it puts a count-shaped reader and a
sentinel-shaped reader in one body with nothing shared between the branches, and
it contradicts the spec's own "classified by its own trait rather than a widening
of an existing one".

**Implement the spec's soundness rule as literally stated** — "fixed-width, or
length driven by the named field". Rejected in §5.1 and reported as §11 F4/F5: it
rejects a schema whose container is driven by a non-named sibling (which is
provably sound, since that container is empty in the terminator), and it admits
the one schema that silently corrupts (a non-zero sentinel on a field that is
another field's length target). This design implements the rule's stated
*purpose* — determined width in the synthesised terminator — plus the obligation
the purpose actually requires.

**Evaluate a `maybe`'s presence predicate at compile time** to admit optionals in
the terminating element. Genuinely possible: a `constexpr` function may build the
default element, set the named field and call `compute_impl`, with the
`std::vector` allocation transient within the evaluation. Rejected because the
predicate reads sibling values that are only default-constructed here, so a
predicate that would be true in a real element and false in the default one makes
the check answer a question nobody asked. Recorded because it is the obvious
relaxation and should be relaxed deliberately if a format ever needs it.

**Resize the containers driven by the named field to the sentinel value**, so a
non-zero sentinel could synthesise a correct non-degenerate terminator. Rejected:
it makes the terminator's size a function of the sentinel (a 255-byte terminator
for `until_field_equals<"size", u8{255}>`), which no format in the scan wants,
and it needs a per-field resize walk on the write path to serve a case with no
witness. §5.3 obligation 5 rejects the schema instead, with a message that says
why.

**Pre-validate the whole vector before emitting any of the run** (the spec's
first open question). Rejected in §3.3 on three counts, the decisive one being
that the guarantee it appears to buy is already false for this construct under
every other error.

**Report a bound-exhausted run as `buffer_exhaustion`, or as
`excessive_length`.** Rejected: the first conflates truncation with corruption,
and the second is documented as meaning nothing was allocated, which is the
opposite of what happened.

**Make the sentinel a constraint object (`until_field_equals<"size", eq{0}>`)**
so the terminator reuses the constraint DSL. Rejected on the spec's own
reasoning: the library already distinguishes constraints the write path can
satisfy from ones it can only check, and admitting the DSL here advertises
`lt{4}` as a terminator, which cannot be constructed. The plain value says
exactly what can be done with it.

---

## 9. Tests

`test/schema/`'s `<feature>_{read,write}.cpp` + `_ct.cpp` convention. Feature
name: `sentinel_records`. Four files: `sentinel_records_read.cpp`,
`sentinel_records_read_ct.cpp`, `sentinel_records_write.cpp`,
`sentinel_records_write_ct.cpp`, registered with `add_struct_cast_test` and
`add_ut_test` respectively.

Read (069), each in both forms:

- a GIF sub-block run: several length-prefixed sub-blocks, a zero-length one,
  and a following field — the terminator is consumed and absent from the vector,
  and the next field reads from the right offset
- a DNS label list, same shape with a different element schema
- a terminator in first position: empty vector, terminator's bytes consumed,
  success
- `max_bytes<N>` accepts a run of exactly `N / sizeof(record)` elements plus its
  terminator, and rejects one element more — the §2.2 placement, and the case
  that fails if the check moves
- stream runs dry between elements: `buffer_exhaustion`
- stream runs dry mid-element: `buffer_exhaustion`, from the nested read
- bound reached with no sentinel: `sentinel_not_found`
- a non-zero sentinel on a named field that is *not* a length target — proves the
  cast-down comparison, and is the case that fails if the sentinel is promoted
  instead (a `signed char` field and a sentinel of `0xff`)

Write (071, 072), both forms:

- a run of real elements plus one synthesised terminator, compared against
  expected bytes
- an empty vector emits the lone terminator, and reads back empty
- an element matching the sentinel in first, middle and last position each fails
  with `found_sentinel_in_sequence`, and `failed_at` names the sequence field
- the last-position case specifically, to pin that there is no "already
  terminated" exemption
- round trip, byte-identical, for GIF and DNS including the empty-run case —
  the feature's acceptance witnesses

Soundness positive control (070), in `test/schema/` rather than
`must_not_compile`: both witness schemas declare and compile, asserted as
`static_assert` on the concept.

`must_not_compile` (070), one program with two cases, registered with
`add_rejected_case`, pinning no text, each checked against a no-`CASE` control
build first:

- `vector_of_records<…, len_from_field<"n">, until_field_equals<"size", u8{0}>>`
  → the existing duplicate-size assertion
- an element containing a `size_from_fields` field →
  `terminator_field_check`'s assertion, with the field named in the instantiation

A third case is worth adding beyond 070's letter, because it is the one that
silently corrupts: a non-zero sentinel on a named field that is another field's
`len_from_field` target → obligation 5.

**One constraint on the compile-time tier that will bite.** The
`sentinel_not_found` case at the *default* bound reads `16 MiB / sizeof(record)`
records — on the order of 10^5 nested `struct_cast_impl` invocations, each
allocating — under constant evaluation. That is past gcc's default
`-fconstexpr-ops-limit` and `-fconstexpr-loop-limit`. Every bound-related `ut`
case must declare a small `max_bytes`, and the default-bound path is exercised in
the GoogleTest suites only. Same limit the companion hit (its F6), for the same
reason, and it needs to be an instruction in 069 rather than a note.

---

## 10. Documentation (073)

`docs/schema/size-axis.md` is the page that changes, and — unlike the companion —
its framing does **not** need re-opening: 068 already rewrote the axis's premise
to admit forms whose length is discovered rather than declared. This adds a
section beside `## until<d>: a length discovered by reading`, something like
`## until_field_equals<field, value>: a run that ends at a sentinel record`.

No new page, so `docs/mkdocs.yml`'s nav is unchanged and
`docs_nav_lists_every_page` passes without edit — 073's nav criterion is
vacuously satisfied, which is worth saying so nobody adds a page to satisfy it.

Content required beyond the table row, each because a reader has no reason to
expect it:

- **the write-side rejection** (072) and the exact property it guarantees: the
  offending element's own bytes never reach the stream, earlier elements' bytes
  do, and there is no rollback. Nothing else in `vector_of_records` constrains
  what an element may contain.
- **both compile-time rejections** (070), as structural rules a schema author
  meets at declaration time — including §6's table of what a terminating element
  may contain, which is the actionable form of the soundness rule.
- **the bound's two consequences**: it counts *in-memory* footprint
  (`n * sizeof(element)`), not wire bytes — different from `until<>`'s
  accounting on the same page, so the difference has to be stated, not implied —
  and it applies per run, so a sentinel run nested inside another has a worst-case
  footprint of the **product** of the bounds, not the outer bound.
- **the three read outcomes and the one write outcome** together, and in
  `docs/errors.md` alongside the enum block, which goes from seven enumerators to
  nine.
- **that the terminating element is consumed and discarded**, so the vector's
  size is the count of real elements and a round trip re-synthesises rather than
  re-emits the terminator.

`docs/schema/index.md`'s descriptor table: the `vector_of_records` row's "Bytes
consumed" cell becomes "element count from the size axis, or — for
`until_field_equals<field, value>` — every element up to and including the first
whose `field` equals `value`", matching how the `str_field` and `vec_field` rows
were amended for `until<>`.

`test/doc_examples/guide_sentinel_terminated_records_example.cpp` — the
GIF-shaped `image_data` with a `blocks` field, bound to its fence with
`<!-- docs: … -->` and `// docs-begin` / `// docs-end`, reading through a binary
`std::ifstream`, defining its own `u8` (the library does not provide the alias
the spec's snippets assume), and returning non-zero on failure.

`README.md` line 246: `- [ ] Read Until a Sentinel Record` → `[x]`.

---

## 11. Findings, contradictions and risks

**F1 — 069 is larger than it reads, and two of its additions are hard errors
rather than missing features.** Neither the spec's "Where it lands" paragraph nor
069's criteria mention `field_list_metadata.hpp` or `field_traits.hpp`'s
`field_like`. `extract_length_dependencies` has no primary definition, so the
first schema containing an `until_field_equals` field fails to compile at
`struct_field_list` — before any reader runs, with a diagnostic pointing at
metadata extraction. `field_like`'s disjunction has the same effect one layer up.
One specialization and one arm (§1.6); both belong in 069, which cannot be
demonstrated at all without them. This is the companion's F1, recurring for the
same structural reason.

**F2 — `size_option_like` must widen, and no issue says so.** The option pack
classifies entries before any descriptor clause is reached, so
`until_field_equals<…>` would be an unrecognised pack entry and
`vector_of_records`'s widened clause would never be consulted. One arm in
`field_options.hpp` (§1.4). Also the companion's F2.

**F3 — `is_vector_of_record_field` has to be narrowed, and two of the three sites
that follow from it fail *silently*.** No issue mentions this, and it is the
riskiest single item in the feature. `census_of_field` keyed on the narrowed
trait returns `{0,0}` for a sentinel-terminated run, which means an announcing
record nested inside one stops being counted and 057's off-spine rejection stops
firing — a schema that is rejected today would start compiling and resolve its
byte order repeatedly. The umbrella concept (§1.5) is the fix, and the three
sites must be changed in the same commit as the narrowing.

**F4 — the spec's soundness rule rejects sound schemas.** "fixed-width, or its
length driven by the named field" excludes a container driven by a *non-named*
sibling. Under default-construct-and-write synthesis that container is empty and
emits nothing, and the sibling driving it derives to zero, so such a schema
synthesises a correct minimal terminator. `{u8 a; u8 size; vec<u8> data
len_from_field<"a">}` terminated on `"size"` emits two zero bytes and round-trips.
§5.2 admits it. Not a contradiction of the spec's *purpose* clause, which this
design implements; a contradiction of its proposed mechanization.

**F5 — the spec's soundness rule admits the one schema that silently corrupts,
and it is the GIF shape with one value changed.** If the named field is another
field's `len_from_field` target — as `"size"` is in the motivating example — the
write path derives its value from that field's contents and discards the stored
sentinel. In the terminator the container is empty, so the byte written is zero.
With a zero sentinel that is correct, which is why GIF works. With
`until_field_equals<"size", u8{0xff}>` on the identical schema, the terminator
goes out as `00`, the reader never sees `0xff`, and the run never ends — reported
as `sentinel_not_found` at the bound if you are lucky and as a misparse of the
rest of the file if the following bytes happen to look like sub-blocks. The
spec's rule passes this schema (`data`'s length *is* driven by the named field).
§5.3 obligation 5 rejects it, and it is worth a `must_not_compile` case beyond
070's letter (§9).

**F6 — 070's contradictory-declaration rejection needs no production code.**
Once F2's arm lands, `until_field_equals` occupies the size slot, so combining it
with a count is two size options and trips `pack_options`'s existing
`"a field takes at most one size option"` assertion. 070's cost for that half is
one test registration and the control-build verification. The issue's framing
implies a rule to be written.

**F7 — the `static_assert` cannot name the offending field, and the issue's open
question presumes it might.** C++23 requires a string literal; P2741's
user-generated messages are C++26 and neither gcc 14 nor the project's baseline
has them under `-std=c++23`. §5.5 settles it the other way: the name reaches the
diagnostic through the *instantiation*, by making the per-field check a per-field
entity. Worth flagging because it also means the wording cannot be tuned to
mention the schema — every sentence has to be generic and the specificity comes
from the template arguments.

**F8 — the compile-time tier cannot exercise the default bound.** §9's last
paragraph. This is a limit of reading records one at a time under constant
evaluation, not a test-writing preference, and 069's "covered in both forms"
criterion has to be read with it: `sentinel_not_found` is covered in both forms
*at a small declared bound*, and at the default bound in GoogleTest only. The
companion's F6, recurring — and worse here, since each iteration is a whole
nested cast rather than one byte.

**F9 — `maybe` wrapping a sentinel-terminated run works, and no slice covers
it.** §8, Extension points. Not a risk so much as an untested capability. One
`maybe` case in 069's suite, or a line in 073's prose, closes it cheaply. The
companion's F9, recurring.

**F10 — the bound's denominator makes the default ceiling much larger in element
terms than the delimited form's, and the docs should say so.** `sizeof(record)`
for a record holding two `std::vector`s is on the order of 50-60 bytes, so the
16 MiB default admits a few hundred thousand elements — each of which may itself
allocate up to *its own* bound. The spec states the nesting product consequence
for nested runs; the flat case deserves the same plainness, because "the bound is
16 MiB" reads as a memory ceiling and is not one.

**F11 — a failure inside an element is reported against the sequence field.**
`write_nested` and `read_field<struct_field_like>` both drop the nested
`failed_at`. This is existing, documented behaviour for every nested record and is
the right answer for 072 (the offending thing is the run), but it means a caller
cannot learn *which element* failed. Worth one sentence in 073 so it is not
discovered as a surprise.

**Nothing in the spec was found to be wrong** other than F4/F5, which concern its
proposed mechanization of a rule whose stated purpose this design implements
unchanged. Its three Open Questions are settled in §3.3, §4 and §5.5.
