# Design: a length can be discovered, not only declared (`until<>`)

Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md` — frozen. This
design consumes it and does not revise it; §10 lists the two places it is
incomplete rather than wrong, and the one place the issues assume machinery that
does not exist.

Issues served, in order: `dev/issues/062-read-a-string-until-a-delimiter.md`
through `068-document-the-discovered-length-size-form.md`. §6 gives the
slice-by-slice content. Two slices change size from how they read (§10): 064 is
smaller, 062 is larger.

Sits on `dev/design/unbounded-resize-from-wire-length.md` (046–047), whose
`max_bytes` this feature repurposes, and inherits verbatim the governing rule
stated in `dev/design/schema-api-verbosity.md`:

> **a class-type NTTP is a non-deduced context in a partial specialization, so
> anything a trait pattern-matches on must live in the value's *type*, never in
> its value.**

The delimiter therefore lives in the type. That is not a style choice; it is the
only way `read_field`, `write_field` and the option classifier can all recover
it.

---

## 0. The shape in one page

| Piece | Where | What |
|---|---|---|
| `delimited_by_t<d>` | `field_size/field_size.hpp` | empty type carrying the delimiter byte as an NTTP |
| `until<d>` | `field_size/field_size.hpp` | the spelling; `inline constexpr auto until = delimited_by_t<d>{}` |
| `delimiter_value_like` | `field_size/field_size.hpp` | one-byte integral — the single-byte restriction, named |
| `is_delimited_size` / `delimited_size_like` | `field_size/field_size.hpp` | the third classification trait, beside `is_fixed_size` and `is_variable_size` |
| `buffer_size_like` | `field_size/field_size.hpp` | `variable_size_like \|\| delimited_size_like` — the widened gate, and the *only* one |
| `byte_buffer_like` | `lib/s2s_traits/type_traits.hpp` | `variable_sized_buffer_like` with a one-byte `value_type` |
| `delimited_buffer_is_byte_wide` | `field/field_options.hpp` | the implication that restricts `vec_field<until<>>` to byte elements |
| `size_option_like` gains an arm | `field/field_options.hpp` | so `until<>` is classified as a size in an option pack at all |
| `is_delimited_field` / `delimited_field_like` | `field/field_traits.hpp` | the field-level trait `read_field` and `write_field` dispatch on |
| `extract_length_dependencies` arm | `field_list/field_list_metadata.hpp` | **not optional** — the primary is undefined (§1.5) |
| `read_delimited<ceiling>` | `field_read/read_impl.hpp` | the per-byte loop, one template, both container types, both stream kinds |
| `read_field<delimited_field_like, F>` | `field_read/field_reader.hpp` | the third read dispatch |
| `write_field<delimited_field_like, F>` | `field_write/field_writer.hpp` | value scan, value, delimiter |
| `error_reason::delimiter_not_found` | `error/cast_error.hpp` | appended (062) |
| `error_reason::found_delimiter_in_value` | `error/cast_error.hpp` | appended (066) |

Four things deliberately **not** touched, each load-bearing:

- `stream/stream_traits.hpp` — no operation added. The per-byte read reuses the
  `read_native_impl` overload pair that already exists (§2.1).
- `field_size/field_size_deduce.hpp` and `comptime_field_size_deduce.hpp` — no
  `deduce_field_size` specialization for a delimited size. Its *absence* is the
  enforcement mechanism for 062's "never instantiated for it" criterion: if any
  path ever reached it, `deduce_field_size<until<...>>` is an incomplete type and
  the build stops. A specialization that returned, say, `0` would satisfy every
  compiler and silently read nothing.
- `field_write/derived_value.hpp` and the length-obligation machinery — a
  delimited field derives nothing and is nothing's length source. This falls out
  of §1.2 rather than needing a rule.
- `field_read/read_impl.hpp`'s `read_impl` byte-order dispatcher — bypassed, for
  the reason in §2.4.

---

## 1. The third size category

### 1.1 The value and its type

```cpp
// field_size.hpp, beside byte_count and size_from_fields_t

// The one thing a delimited field declares. Like size_from_fields_t and unlike
// byte_count, the payload lives in the type: read_field, write_field and the
// option classifier all pattern-match on it.
template <unsigned char d>
struct delimited_by_t {
  static constexpr unsigned char delim = d;
};

// A delimiter is one byte. Named rather than written inline as a requires
// clause so the diagnostic for until<u16{0x0d0a}> says what the rule is, and so
// a multi-byte form can later relax it in one place.
template <typename T>
concept delimiter_value_like = std::is_integral_v<T> && sizeof(T) == 1;

template <delimiter_value_like auto d>
inline constexpr auto until = delimited_by_t<static_cast<unsigned char>(d)>{};
```

`unsigned char` is the canonical form of the delimiter throughout: the schema may
spell it as `u8{0}`, `'\0'` or `char{0}`, and all three land on the same
`delimited_by_t<0>`, so two schemas that mean the same delimiter are the same
type. Comparison against a buffer element is done by casting the delimiter to the
element type *once*, never by promoting the element (§2.2) — the difference
matters for a delimiter ≥ `0x80` in a `std::string`, where `char` is signed on
every platform this library targets.

`delimiter_value_like` admits `char`, `signed char`, `unsigned char`, `char8_t`
and `bool`. `until<true>` meaning "stop at byte 1" is absurd but harmless, and
excluding it would cost a special case for a spelling nobody writes.

The delimiter is read back as `size_type_of<T::field_size>::delim`, mirroring
`size_type_of<size>::f` on a computed size. No `delimiter_of<S>` accessor is
introduced; `len_source_of` exists only because `field_accessor` predates the
convention, and copying it here would add an indirection with nothing behind it.

### 1.2 Why it must not satisfy `variable_size_like` — and what that buys for free

`is_variable_size` is not a description, it is a routing decision. Everything
keyed on it consumes a *count*:

| Site | What it does with a `variable_size_like` size |
|---|---|
| `field_size_deduce.hpp:18` | resolves it to a `std::size_t` |
| `field_reader.hpp:39` | hands that count to `read_impl` to resize by |
| `field_writer.hpp:76` | compares it against `value.size()` |
| `derived_value.hpp:26,90` | inverts it to derive the length slot |
| `field_list_metadata.hpp:61,269` | records the named field as a length dependency |

A delimited field has nothing to give any of them. Keeping it out of
`is_variable_size` therefore does more than avoid one bad path: it makes all five
sites correct by construction. In particular it is the whole of 065's "No entry
is added to the derived-field obligations" — that is not a rule the write path
enforces, it is a consequence of a trait not matching.

The categories are disjoint by construction (`is_variable_size` has no
specialization for `delimited_by_t`, `is_delimited_size` none for
`field_accessor` or `size_from_fields_t`), so no `read_field`/`write_field`
partial specialization is ambiguous and none needs a negative constraint of the
`&& !is_computed_size_v<...>` kind the length-derivation sites carry.

### 1.3 The gates that widen, and the ones that must not

One new concept, used at exactly two descriptors:

```cpp
// field_size.hpp
// The sizes a resizable byte buffer may declare: a count resolved before the
// read, or a delimiter that ends it. Deliberately a *new* concept rather than a
// widening of variable_size_like, which is the gate into deduce_field_size.
template <typename T>
concept buffer_size_like = variable_size_like<T> || delimited_size_like<T>;
```

| Site | Today | After | Why |
|---|---|---|---|
| `field_descriptors.hpp:82` `str_field` | `variable_size_like` | `buffer_size_like` | 062 |
| `field_descriptors.hpp:66` `vec_field` | `variable_size_like` | `buffer_size_like` + `delimited_buffer_is_byte_wide` | 063, §4 |
| `field_descriptors.hpp:73` `vector_of_records` | `variable_size_like` | **unchanged** | a record is not a byte; the delimited loop reads bytes |
| `type_tags.hpp:84,93,111` `as_vector`/`as_string`/`as_vector_of_records` | `variable_size_like` | **unchanged** | a delimited union alternative is out of scope; the existing clause rejects it cleanly, as it already rejects an unsized one (`must_not_compile` CASE 6) |
| `field_options.hpp:36` `size_option_like` | 4 arms | **+ `delimited_size_like`** | §1.4 |
| `field_size.hpp` `atomic_size` | — | **unchanged** | `atomic_size` exists only to constrain `size_choices`, which is not declarable (docs, `dev/issues/027`) |
| `field_size.hpp` `is_size_like` | 3 arms | **+ `delimited_size_like`** | it is the "is this on the size axis" predicate, and the spec's own criterion is that `until<>` is *on* the axis. Currently unused, which is exactly why omitting it would be found later rather than now |

The widening admits nothing that was previously rejected: `buffer_size_like` is a
strict superset of `variable_size_like` by exactly `delimited_size_like`, and
`delimited_size_like` is satisfied by one type that does not exist today.

### 1.4 The option pack, and why `basic_field` gains a constraint

`until<u8{0}>` arrives as a trailing entry in a descriptor's option pack. Pack
entries are classified per element by `field_options.hpp`'s concepts, so without
an arm in `size_option_like` the spelling is not a size, not a constraint, not a
bound — it is an unrecognised entry, and `str_field<"kw", until<u8{0}>>` fails on
the placeholder constraint before any of §1.3's widening is consulted.

So `size_option_like` gains `|| delimited_size_like<S>`. Everything downstream
then works untouched: `size_in_pack` picks it as the field's size,
`size_option_count` counts it, so `str_field<"kw", until<u8{0}>, 4_B>` trips the
existing `"a field takes at most one size option"` assertion with no new rule.
That is the payoff for treating `until<>` as a size rather than as a fourth
option kind beside `max_bytes` — see §7's rejected alternatives.

The cost is one leak: `basic_field` takes `field_option_like`, which includes
`size_option_like`, so `basic_field<"x", u32, until<u8{0}>>` now classifies and
proceeds to `field_fits_to_underlying_type`, whose body is
`deduce_field_size<size>{}() <= sizeof(field_type)` — an incomplete type in an
atomic constraint. Whether that is a substitution failure or a hard error is a
question about the immediate context that is not worth relying on, and either
diagnostic is about arithmetic rather than about the rule.

**`basic_field` therefore gains `fixed_size_like` ahead of the fits-check:**

```cpp
template <fixed_string id, integral T, field_option_like<T> auto... opts>
  requires fixed_size_like<size_type_of<size_of_pack<T, opts...>>> &&
           field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
using basic_field = ...;
```

Conjunction short-circuits during constraint checking, so the fits-check is never
substituted for a non-fixed size. This rejects nothing that compiles today — a
`len_from_field` or `size_dont_care` on a `basic_field` already fails on the same
fits-check, only later and less legibly. It is 064's "`until<>` on `basic_field`
fails to compile", and it is the only line of production code 064 needs.

`fixed_array_field`, `c_str_field`, `fixed_string_field`, `c_arr_field`,
`magic_*`, `struct_field`, `variance` and `announces_byte_order` take
**constraint options only**. `until<>` on any of them is already an unrecognised
pack entry today and stays one; 064's remaining rejections need no code, only
test entries (§10, finding F5).

### 1.5 The hard error no issue mentions

`field_list_metadata.hpp`'s `extract_length_dependencies` has **no primary
definition** — deliberately, per its own comment: "a wrapper that reaches no
specialization is a hard error rather than a silently empty answer". Its four
field-shaped specializations are constrained on `fixed_size_like`,
`size_dont_care_like`, `variable_size_like && !computed`, and `is_computed_size_v`.

A field carrying a delimited size matches none of them. `struct_field_list`
instantiates `field_list_metadata<fields...>` for every schema, so **the first
schema declaring an `until<>` field fails to compile with an incomplete-type
error before any of this feature's read or write code is reached.**

```cpp
template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires delimited_size_like<size_type_of<size>>
struct extract_length_dependencies<field<id, T, size, constraint, bound>> {
  // A delimiter is carried in the field's own declaration, so a delimited field
  // depends on no sibling — the same answer a fixed size gives, for the same
  // reason, and stated for the same reason the size_dont_care arm is.
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};
```

Likewise `field_traits.hpp`'s `field_like` disjunction must gain
`delimited_field_like`, or `all_field_like` rejects the schema at
`struct_field_list`. Both belong to 062 (§10, finding F1).

### 1.6 The field trait and the read dispatch

```cpp
// field_traits.hpp, beside is_variable_sized_field
template <fixed_string id, byte_buffer_like T, auto size, auto constraint_on_value, auto bound>
  requires delimited_size_like<size_type_of<size>>
struct is_delimited_field<field<id, T, size, constraint_on_value, bound>> {
  static constexpr bool res = true;
};
```

`byte_buffer_like` rather than `variable_sized_buffer_like` is §4's restriction,
stated once at the engine level and reused at the descriptor level so the two
cannot drift.

```cpp
// field_reader.hpp
template <delimited_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;

  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list) {}

  // The order cell is unnamed, not ignored: a delimited field's elements are
  // one byte wide, so there is no byte order to apply. That is the same fact
  // that lets this reader bypass read_impl entirely.
  template <typename stream>
  constexpr auto read(stream& s, cast_endianness&) const -> rw_result {
    constexpr auto delim = size_type_of<T::field_size>::delim;
    return read_delimited<bound_in_bytes<T::field_bound>>(s, field.value, delim);
  }
};
```

`field_list` is unused here — the delimited reader consults no sibling — but the
member stays, because `struct_cast_impl` constructs every `read_field` the same
way. It is the only reader with an unused member, and that is the honest signal
that this size is the one resolved from nothing but the stream.

---

## 2. The read

### 2.1 One loop, two container types, two stream kinds, no new stream operation

```cpp
// read_impl.hpp, beside read_native

// Per byte, by decision: the spec's format scan puts every delimited field at
// tens of bytes, and the alternatives all change the stream concept or need a
// pushback buffer threaded through every field reader. read_until is recorded
// in the spec as the fast primitive and is deliberately not built.
template <std::size_t ceiling, byte_buffer_like T, input_stream_like stream>
constexpr auto read_delimited(stream& s, T& obj, unsigned char delim) -> rw_result {
  using element = typename T::value_type;
  // Cast the delimiter down once, rather than promoting each element up: for a
  // std::string and a delimiter >= 0x80 the promoted comparison is false for
  // the very byte that should match.
  const element stop = static_cast<element>(delim);

  obj.clear();
  while(true) {
    element byte{};
    // The one place the two stream shapes coincide. read_native_impl's
    // constexpr overload stages through std::array<char, sizeof(T)> and its
    // runtime overload through char*, and at sizeof(element) == 1 both are a
    // single byte — so the existing overload pair is the whole of the
    // constexpr/runtime split, and this loop needs no `if constexpr` of its own.
    auto res = read_native_impl(s, byte, sizeof(element));
    if(!res)
      return res;                                  // buffer_exhaustion
    if(byte == stop)
      return {};                                   // consumed, not stored
    if(obj.size() == ceiling)
      return std::unexpected(error_reason::delimiter_not_found);
    obj.push_back(byte);
  }
}
```

`std::string` and `std::vector<u8>` differ only in `T::value_type`, so 063 adds
no code to this function — it instantiates it a second time. That is 063's "the
read loop is shared with 062's, not duplicated", discharged by the template
rather than by a convention someone has to keep.

`obj.clear()` is not defensive: it states that the function replaces the
container's contents rather than appending to them. Without it the function would
be correct only for the callers that happen to pass a freshly constructed field.

**No `reserve`.** Reserving `ceiling` would allocate the very 16 MiB the default
bound exists to prevent, on every delimited field. `push_back`'s geometric growth
costs at most a factor of two over the value's real length, which for a
tens-of-bytes field is nothing.

### 2.2 Where the bound is enforced (spec question 3)

**`checked_byte_count` does not apply here, and a per-iteration comparison
replaces it.** The reasoning, since the two look interchangeable:

`checked_byte_count` exists to validate *a length that came off the wire, before
allocating by it*, and is phrased as a division precisely so the unvalidated
product is never evaluated. A delimited read has no such length. The container
grows one element at a time under a loop that cannot pass `ceiling`, so there is
no product to overflow and nothing to validate ahead of time. Calling it would be
arithmetic on a number the loop already controls.

The two also report different things, and that is not cosmetic. `excessive_length`
means "a declared length was never satisfiable, and nothing was allocated" —
`docs/errors.md` says so in as many words. Reaching the bound here means the
opposite: everything up to the bound *was* read and allocated, and the delimiter
was not among it. That is `delimiter_not_found`.

The comparison's placement is the one subtle part. The bound counts **the value,
not the wire**, so `max_bytes<79>` must accept a 79-byte keyword *plus* its
delimiter. Checking `obj.size() < ceiling` as a loop condition gets this wrong by
one: after pushing the 79th byte the loop exits and reports
`delimiter_not_found` without ever reading the delimiter that was sitting there.
The check therefore sits **after the delimiter test and before the push**: the
loop reads at most `ceiling + 1` bytes, and the `ceiling + 1`-th is only ever
consumed to discover that it is not the delimiter. `max_bytes<79>` accepts a
79-byte value and rejects an 80-byte one, which is what 062 asks for and what
lets a schema author transcribe PNG's stated 1–79 unadjusted.

### 2.3 The three outcomes

| Outcome | Reported as | Raised by |
|---|---|---|
| delimiter found | success; delimiter consumed, not stored | the loop |
| stream ran dry first | `buffer_exhaustion` | `read_native_impl`, unchanged |
| bound reached first | `delimiter_not_found` (new) | the loop |

`buffer_exhaustion` arrives from the existing leaf without a line of new code,
which is the right outcome: "the stream ran dry" is the same fact here as
everywhere else and should not acquire a second spelling. A delimiter in first
position returns success with an empty container, having consumed one byte — the
ELF string-table index 0 case, and not an error, since a minimum length is a
constraint a schema author writes.

### 2.4 Why the reader bypasses `read_impl`

`read_impl`'s entire job is choosing a byte-order strategy. A delimited field's
elements are one byte wide (§4), so both of its branches would reach the same
bytes: `read_native` and `read_foreign_buffer` differ only by
`byteswap_elements`, which `byte_order.hpp` already documents as a no-op on
single-byte elements. Routing through it would mean threading a delimiter
parameter through a function whose three other callers have none, to select
between two identical answers.

The same argument applies in the other direction (§3.1) and to the same effect,
and it is the *reason* §4's restriction is worth having rather than a consequence
of it.

---

## 3. The write

### 3.1 The specialization

```cpp
// field_writer.hpp
template <delimited_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <typename stream>
  constexpr auto write(stream& s, cast_endianness&) const -> rw_result {
    using element = typename T::field_type::value_type;
    constexpr element stop = static_cast<element>(size_type_of<T::field_size>::delim);

    // Before any byte leaves: a rejected value leaves the stream untouched,
    // which is the rule write_variant_impl already states — a discarded value
    // is recoverable, half a field is not.
    if(find_index(value, stop) != value.size())
      return std::unexpected(error_reason::found_delimiter_in_value);

    // Empty is a valid value and emits the lone delimiter. Skipped rather than
    // written as a zero-length run: const_byte_addressof on an empty vector
    // yields data(), which may be null.
    if(!value.empty()) {
      auto res = write_native(s, value, value.size());
      if(!res)
        return res;
    }
    return write_native_impl(s, stop, sizeof(stop));
  }
};
```

`find_index` is `lib/algorithms/algorithms.hpp`'s, reused rather than
reimplemented; it is `constexpr`, range-based, and returns `size()` on no match.
It is safe here for the reason §2.1 gives — `stop` is already the element type,
so the comparison is never a promotion.

`write_native` rather than `write_impl`, and `write_native_impl` for the single
delimiter byte, for §2.4's reason; `write_foreign_buffer`'s own code already
routes single-byte elements to `write_native_impl`, so this is the branch it
would have taken anyway, chosen statically instead of at run time.

Neither `is_derived_target_v` nor `is_frozen_target_v` is consulted: a delimited
field is neither. It cannot be frozen (`frozen_field_like` requires
`fixed_sized_field_like`), and it cannot be derived (§1.2). No branch says so;
the traits simply do not match.

`failed_at` naming the offending field is free — `stream_cast_impl`'s fold
attaches `fields::field_id` to every `rw_result` failure a writer returns. 066
needs nothing for it beyond a test.

### 3.2 Where the delimiter check fires (the spec's open question, settled)

**Decision: one check, in the writer, evaluated as ordinary code — which means it
fires at compile time for a constexpr write and at run time otherwise, without
being written twice or behaving differently.**

The question as the spec poses it ("a constexpr write of a literal could reject
at compile time; a runtime value cannot") contains a hidden premise worth
disputing: that a compile-time rejection means a *diagnostic*. It does not have
to. Under `ut`'s compile-time mode the whole write is a constant expression, so
the check above executes during constant evaluation and produces a
`std::unexpected(found_delimiter_in_value)` at compile time. The test asserts on
that value at compile time. Nothing is deferred.

Making it a diagnostic instead — a `throw`, or any other non-constant operation,
so that a bad constexpr write fails to compile — was considered and rejected on
three counts:

1. It breaks the project's error model. AGENTS.md's first override is
   `std::expected` throughout; a write that fails differently depending on where
   it runs is two error models.
2. It destroys the test. `dev/design/compile-time-test-tier.md`'s whole premise
   is one source compiled three ways, the same assertions in each. A failure
   that is a hard error at compile time and a value at run time cannot be
   asserted on in both.
3. It buys nothing. The value is `const typename T::field_type&` — a function
   parameter, not a template argument — in both modes. There is no "literal" for
   the compiler to see that it does not already see.

A genuine compile-time rejection *is* available in one exotic corner: a schema
that pins a delimited field with `eq{...}`, where the pinned value is an NTTP and
could be scanned for the delimiter at declaration time. Rejected as well: it
covers one spelling nobody has written, and the trait would have to understand
every constraint kind well enough to decide which ones pin a value. The
evaluated check already covers it, correctly, on the first write.

### 3.3 The rule has no "already terminated" exemption

A value whose last byte is the delimiter is rejected like any other, because the
writer emits the delimiter itself and would otherwise emit a second one. This
falls out of the scan being over the whole value — there is no trailing-byte
special case to write, and adding one would be the bug.

---

## 4. `until<>` on `vec_field` restricts elements to byte width (063's open question, settled)

**Decision: yes. `vec_field<"data", u32, until<u8{0}>>` is a compile error.**

Three reasons, in increasing order of how much they cost to get wrong:

1. **The match is undefined otherwise.** A delimiter is one byte. Against a `u32`
   element, "matched" could mean the whole element equals `0x00000000`, or any
   byte of it is zero, or a match straddles two elements. No format in the
   spec's scan asks the question, so any answer would be invented.
2. **It would drag byte order back in.** A multi-byte element read per byte has
   to be byteswapped before it means anything, so "did it match" would depend on
   the order cell — and the order cell can be resolved by a byte-order-announcing
   record earlier in the same file. §2.4's and §3.1's bypasses both stop being
   sound, and `announcing_record.hpp`'s order-agnosticism answer for the field
   changes with its element type.
3. **It would reintroduce the multiply.** The bound counts bytes. With byte
   elements `obj.size()` *is* the byte count and §2.2's check is a comparison of
   two `size_t`s. With wider elements it becomes `size * sizeof(element)` against
   the ceiling, per iteration — the product `checked_byte_count` is phrased as a
   division to avoid.

Expressed as one predicate with two users, so the engine and the descriptor
cannot drift:

```cpp
// lib/s2s_traits/type_traits.hpp, beside variable_sized_buffer_like
template <typename T>
concept byte_buffer_like =
  variable_sized_buffer_like<T> && sizeof(typename T::value_type) == 1;

// field/field_options.hpp
// A delimiter is one byte, so a delimited field's elements are one byte.
// Phrased as an implication over the pair rather than folded into
// buffer_size_like: a schema declaring neither half should never see this rule.
template <typename S, typename buffer>
concept delimited_buffer_is_byte_wide = !delimited_size_like<S> || byte_buffer_like<buffer>;
```

Used on `vec_field` (`field_descriptors.hpp:66`) and by `is_delimited_field`
(§1.6). **Not** used on `str_field`: `std::string::value_type` is `char`, so the
implication is vacuously true there and stating it would be noise.

Consequences worth writing down:

- `vec_field<"d", std::array<u8,4>, until<u8{0}>>` is rejected too —
  `is_vector_like` admits fixed-array elements, and `sizeof` catches them.
- The rejection is a named concept, so the diagnostic reads
  `delimited_buffer_is_byte_wide<delimited_by_t<0>, std::vector<unsigned int>>`.
  That satisfies 063's "a diagnostic naming the reason" through the concept's
  name, which is the mechanism AGENTS.md's "concepts, not SFINAE" override
  prescribes and the one `must_not_compile` already relies on (§8).

---

## 5. A discovered length feeding a computed one (067) — checked, not assumed

**`compute_impl` already forwards a non-integral sibling. 067 is a test and a doc
example; no production code changes.**

Verified rather than inferred, at
`field_compute/computation_from_fields_impl.hpp`:

- `compute_impl::operator()` is `callable(field_value_of<field_accessor<req_fields>>(flist)...)`.
  There is no conversion, no `static_cast`, no integral constraint anywhere in
  the expansion.
- `field_value_of` (`field_list/field_list.hpp:113`) returns
  `const meta::type_of<...>&` — the field's own type by reference, whatever it is.
- The gate is `can_eval_R_from_fields`, i.e.
  `std::is_invocable_r_v<std::size_t, decltype(callable), decltype(field_value_of<...>)...>`.
  A callable taking `(u32, const std::string&)` and returning `std::size_t`
  satisfies it.

The PNG shape therefore composes today:

```cpp
s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>,
```

Two further checks, both clean:

- **Dependency resolution.** `extract_length_dependencies` for the *text* field
  takes the computed-size arm and records `"length"` and `"keyword"`; both are
  declared earlier, so `size_dependencies_resolved` passes. The *keyword* field
  takes §1.5's new arm and records nothing. Without §1.5 this slice cannot even
  be attempted, which is a second reason that arm belongs in 062.
- **Write-side assignability.** `extract_unconditional_len_sources` fires only
  for a non-computed `variable_size_like` size, so naming `"keyword"` in a
  `size_from_fields` does **not** make it a derived field. It stays assignable,
  which is what the round-trip needs. The `"text"` field's computed size is
  verified against the container with `found_contradicting_length`, exactly as it
  is for any other computed size — a caller whose `length` disagrees with
  `keyword.size() + 1 + text.size()` is told so.

---

## 6. Slice-by-slice

**062 — `str_field` reads until a delimiter.** `delimited_by_t`, `until<>`,
`delimiter_value_like`, `is_delimited_size`/`delimited_size_like`,
`buffer_size_like`, the `size_option_like` arm, `byte_buffer_like`,
`is_delimited_field`/`delimited_field_like`, the `field_like` arm, the
`extract_length_dependencies` arm, `read_delimited`, the `read_field`
specialization, `str_field`'s widened clause, `error_reason::delimiter_not_found`.
Larger than the issue reads — see §10 F1.

**063 — `vec_field`.** `vec_field`'s widened clause plus
`delimited_buffer_is_byte_wide`. `read_delimited` instantiates a second time;
nothing is added to it. One `must_not_compile` entry for a wide element type.

**064 — rejections.** One line of production code (`basic_field`'s
`fixed_size_like`, §1.4). Everything else is test registration: the multi-byte
delimiter is rejected by `delimiter_value_like`, which 062 introduced;
`fixed_array_field` and `c_str_field` reject `until<>` as an unrecognised pack
entry, which is today's behaviour for any non-constraint option on them.

**065 — the write path.** The `write_field` specialization *minus* the scan, i.e.
the last five lines of §3.1. Must land with 066 in one reviewable unit — between
them the branch writes values that read back short, which is precisely the hazard
the feature exists to prevent.

**066 — the rejection.** The scan, `error_reason::found_delimiter_in_value`, and
the `docs/errors.md` entry.

**067 — composition.** Tests and a doc example only (§5).

**068 — documentation.** §9.

---

## 7. Decisions, stated explicitly

### State and data lifecycle

Nothing in this feature is transactional, and one thing had to be decided about
partial completion.

**On the read side, the container is the unit.** `read_delimited` clears and then
grows the caller's container; on failure it returns with the container holding
whatever it had accumulated. That is deliberate and matches every other reader:
`struct_cast_impl` discards the whole `struct_field_list` on the first failure, so
a partially filled field is never observable by anyone. Writing the loop to build
into a temporary and swap on success would buy nothing and would allocate twice.

**The stream position after a failure is not restored.** It cannot be — the stream
concept has no seek, deliberately — so the position after a
`delimiter_not_found` is `ceiling + 1` bytes past where the field started. This
is already true of every failing read in the library, and it is why failure is
terminal for the whole cast rather than recoverable per field.

**On the write side, the unit is the field, and it is genuinely enforced.** The
delimiter scan runs before any byte is emitted (§3.1), so a rejected value leaves
the stream byte-identical to before the field. This is the one place in the
feature where partial completion would be observable and irreversible — a value
written half-way followed by an error leaves a file that reads back as something
else — and it is why the ordering is stated rather than incidental.

Nothing here is replayable or auditable, and nothing needs to be: a cast is a
pure function from a stream to a value.

### Error propagation

Unchanged in structure; two new leaves.

The layering stays exactly as it is: `read_delimited` and the writers return
`rw_result` carrying a bare `error_reason` and knowing nothing about field names;
`struct_cast_impl`/`stream_cast_impl` attach `failed_at` from `fields::field_id`.
That is the one seam where the representation changes, and this feature adds no
second one.

Two enumerators, both **appended** so existing values stay stable, per the
precedent `found_contradicting_length` and `excessive_length` set:

- `delimiter_not_found` — the bound was reached with no delimiter. New because
  `buffer_exhaustion` would be diagnostically useless here: truncated and corrupt
  are different facts about a file and a caller can act differently on them.
- `found_delimiter_in_value` — the write-side rejection. Named for
  `found_contradicting_length`'s shape, and deliberately not folded into
  `validation_failure`: the author has not violated a constraint they wrote, they
  have hit a rule of the size form.

The layer below is reused rather than wrapped: `buffer_exhaustion` reaches the
caller from `read_native_impl` untouched.

### Concurrency and ownership

**Single-threaded, no shared mutable state, no change.** Every object this
feature touches is owned by one call chain: the `struct_field_list` belongs to
`struct_cast_impl`'s frame, the container belongs to the field, the stream belongs
to the caller and is borrowed by reference for the duration of the cast. The one
piece of state that outlives a single field — the `cast_endianness&` cell
threaded through the readers — is not written by anything in this feature and is
not even named by it (§1.6).

The per-byte loop makes the stream's position mutate `ceiling + 1` times where
other readers mutate it once. That is a difference in frequency, not in kind: it
is the same single borrow, and two concurrent casts over one stream were never
supportable and still are not.

### Reuse

Used rather than reimplemented:

- **`read_native_impl`'s overload pair** — the whole constexpr/runtime split for
  the read loop (§2.1). This is the decision that makes the loop shared rather
  than doubled, and it works only because a one-byte read is the point where
  `std::array<char, N>&` and `char*` coincide.
- **`find_index`** (`lib/algorithms/algorithms.hpp`) for the write-side scan.
- **`max_bytes` / `bound_in_bytes` / the `field` bound parameter** — the entire
  bound vocabulary from 046–047, repurposed rather than paralleled.
- **`byte_buffer_like` as one predicate with two users** (§4), so the engine's
  trait and the descriptor's constraint cannot drift.
- **`push_back` growth** rather than the project's own containers: the field type
  is `std::string`/`std::vector` because the schema says so, and `lib/`'s
  containers are for template-facing code, which this is not.

Introduced here and worth reusing later:

- **`read_delimited`** is the incremental-read machinery the companion brief
  (`a-record-sequence-cannot-end-at-a-sentinel.md`) needs. It is deliberately a
  free function over `(stream, container, delimiter)` rather than a member of
  anything, so the sentinel-terminated record sequence can call it or copy its
  shape without inheriting a field reader.
- **`delimited_size_like` as a classification rather than a special case** is what
  lets the sequence feature add `until_field_equals` as a fourth category beside
  it rather than as an exception inside this one.

Deliberately *not* reused: `checked_byte_count` (§2.2), and `s2s::integral` for
`delimiter_value_like` — the latter would pull `type_traits.hpp`'s `<vector>`,
`<variant>` and `<optional>` into `field_size.hpp`'s closure to save writing
`std::is_integral_v`, and `field_size.hpp` is a leaf worth keeping leafy.

### Extension points

Three, each with the contract that holds it stable:

- **A multi-byte delimiter** relaxes `delimiter_value_like` and adds a
  `delimited_by_sequence_t`. The contract is that `until<u8{0}>` already names
  its argument as a value rather than a pack, so the single-byte spelling does
  not change when a sequence spelling is added. This is why the restriction lives
  in a named concept on the spelling rather than in `delimited_by_t`'s parameter
  type.
- **`read_until` as a stream operation**, recorded by the spec and not built. The
  contract is that `read_delimited` is the only caller of `read_native_impl` in a
  loop, so an `if constexpr(has_read_until<stream>)` fast path goes in one
  function and every existing custom stream keeps compiling on the fallback. The
  concept change would be additive; the cost that keeps it out is testing four
  configurations for one field kind.
- **`until<>` behind `maybe`** already works and is not in any slice's scope.
  `to_optional_field_v` rewrites the field type to `std::optional<std::string>`,
  which matches `is_optional_field` and neither of the buffer traits, and
  `read_field<optional_field_like>` delegates to the base field. Recorded because
  it is the kind of thing discovered by a user before a maintainer.

Explicitly **not** extension points: union alternatives (`type_tags.hpp` keeps
`variable_size_like`), `vector_of_records`, and runtime-valued delimiters. Each
is a spec Non-Goal, and each is rejected today by an existing clause rather than
by a new one.

### Build vs. buy

No new dependency is possible or wanted: this is a header-only library amalgamated
into a single shipped header, and every component here is either a trait or a
loop over a stream.

The one real build-vs-buy question is the **read primitive**, and it was
genuinely evaluated, by the spec, against three existing facilities:

- `std::istream::getline(dest, n, delim)` has exactly the right semantics —
  stops at the delimiter, consumes it, does not store it. It cannot be used
  because it is `std::istream`'s and the stream concept is not `std::istream`;
  reaching it would mean adding an operation to the concept, which the spec
  forbids for this feature.
- `std::istream::peek()` was evaluated and rejected on cost: it constructs a
  sentry exactly as `read` does, so peeking a byte at a time costs what reading a
  byte at a time costs, with no lookahead spelling for K bytes that works on a
  pipe.
- Chunked reading is worse: a delimiter at offset 9 of a 64-byte read leaves 54
  consumed bytes belonging to the next field, unreturnable without seek or unget,
  so a pushback buffer would have to be threaded through every field reader's
  signature.

So: build the per-byte loop, and record `read_until` as the bought-later
primitive. `find_index` is the one place something existing was taken instead of
written.

### Abstractions introduced

Each with the problem that forces it. Anything below that lacks one has been
collapsed.

| Introduced | Forcing problem |
|---|---|
| `delimited_by_t<d>` | a trait must pattern-match the delimiter, and a class-type NTTP is a non-deduced context in a partial specialization |
| `delimited_size_like` | routing: three size categories now need telling apart, and `is_fixed_size`/`is_variable_size` are already the shape for it |
| `buffer_size_like` | two descriptors must admit exactly one more thing, and `variable_size_like` cannot be the place to say it (§1.2) |
| `delimiter_value_like` | 064 needs the single-byte restriction to carry a name a reader learns the rule from |
| `byte_buffer_like` | §4's restriction has two enforcement sites and must not drift between them |
| `delimited_buffer_is_byte_wide` | the rule is about the *pair* (size, element), and a schema declaring neither half should not see it |
| `is_delimited_field` / `delimited_field_like` | `read_field` and `write_field` dispatch on field traits, not on size traits |
| `read_delimited` | the loop has two instantiations (string, vector) and one caller per direction |
| `error_reason::delimiter_not_found` | truncated and corrupt are different facts about a file |
| `error_reason::found_delimiter_in_value` | a size-form rule is not a schema author's constraint, and `validation_failure` would say it was |

Collapsed rather than introduced, each having failed the same bar:

- **`delimiter_of<S>`** — `size_type_of<S>::delim` already reads it, as
  `size_type_of<S>::f` does for a computed size.
- **A `delimited_field_options` pack kind** beside `bound_pack_option_like` —
  `until<>` *is* a size, so `size_in_pack` and the duplicate-size assertion serve
  it unchanged (§1.4).
- **A `deduce_field_size` specialization returning 0 or an `optional`** — its
  absence is the enforcement (§0).
- **A delimited arm in `read_impl`/`write_impl`** — §2.4.
- **A `scan_for_delimiter` helper** — one call to `find_index`.

### Alternatives rejected

**Widen `variable_size_like` to include `until<>` and special-case
`deduce_field_size`.** The smallest diff, and it inverts the dependency: every
one of §1.2's five consumers would need a "unless it is delimited" branch, and
each branch would be a place to forget. Rejected because the five correct
behaviours are free if the trait simply does not match, and conditional if it
does.

**Make `until<>` a fourth option kind, admitted only by two descriptors, on
`max_bytes`'s precedent.** Genuinely tempting — `bound_pack_option_like` exists
for exactly this shape. Rejected because a bound is not a size and a delimiter is:
`until<u8{0}>` occupying the size slot is what makes `until<u8{0}>, 4_B` a
duplicate-size error for free, what makes `size_of_pack` and `field::field_size`
carry it with no new plumbing, and what makes the spec's claim — that this is a
member of the size axis rather than a special case beside it — true in the code
rather than only in the prose.

**Give `read_delimited` an `if constexpr(identified_as_constexpr_stream<stream>)`
split, mirroring `read_native`'s variable-sized overload.** Rejected on
inspection: `read_native` needs the split because a `std::vector` cannot be
`bit_cast` during constant evaluation, so the bulk read has to become an element
loop. A per-byte loop is *already* the element loop, and at one byte the two
staging shapes coincide (§2.1). Writing the split anyway would double the loop
and leave two places to fix a bound off-by-one.

**Reserve `ceiling` before the loop.** Rejected: a 16 MiB allocation per
delimited field, to avoid a growth factor of two on a 79-byte value.

**Enforce the bound with `checked_byte_count`.** Rejected in §2.2: no
wire-supplied length exists, the product it guards cannot occur, and it reports
the wrong fact.

**Make a constexpr write of a delimiter-bearing value a hard diagnostic.**
Rejected in §3.2 on three counts, the decisive one being that it makes the
failure untestable in the compile-time tier.

**Restrict `until<>` to `str_field` only and leave `vec_field` out.** Rejected by
the spec's Non-Goals, which claim delimited runs of trivials for this feature to
stop the companion brief growing a second spelling for them. Recorded because the
implementation cost is one requires-clause, so the decision is entirely about the
declaration surface.

**Add `read_until` to the stream concept now.** Rejected for this feature, not
forever: it multiplies the test matrix by four (fast and fallback, each constexpr
and runtime) for one field kind, and the fallback has to exist and be tested
regardless.

---

## 8. Tests

Placement follows `test/schema/`'s `<feature>_{read,write}.cpp` + `_ct.cpp`
convention. Feature name: `delimited`. New files:
`delimited_read.cpp`, `delimited_read_ct.cpp`, `delimited_write.cpp`,
`delimited_write_ct.cpp`.

Read (062, 063) — each in both forms:

- a NUL-terminated keyword followed by a known field; the delimiter is consumed
  and absent from the value
- a delimiter in first position: empty value, one byte consumed, success
- `max_bytes<79>` accepts a 79-byte value plus its delimiter and rejects an
  80-byte one — the §2.2 off-by-one, and the case that fails if the bound check
  moves
- stream runs dry before the delimiter: `buffer_exhaustion`
- bound reached before the delimiter: `delimiter_not_found`
- the same five against `vec_field<"d", u8, until<u8{0}>>` (063)
- a delimiter ≥ `0x80` on a `str_field`, which is the §2.1 sign trap and passes
  trivially with the cast and fails without it

Write (065, 066) — both forms:

- value plus one delimiter byte, compared against expected bytes
- empty value emits the lone delimiter, and reads back empty
- a value containing the delimiter fails with `found_delimiter_in_value`, and
  `failed_at` names the field
- a value whose *last* byte is the delimiter is rejected too
- round trip for every read case above

Composition (067): the PNG tEXt shape, read and round-tripped, in both forms,
with the keyword at 79 bytes.

`must_not_compile` (063, 064), one target per program per that directory's
convention, each registered with `add_rejected_case` and — per that file's own
header comment — asserting only that the build fails, since concept wording
differs across gcc, clang and MSVC. Each must be checked against a no-`CASE`
control build and confirmed to fail on *its own* diagnostic before registration,
which is the trap 047 fell into:

- `until<u16{0x0d0a}>` (multi-byte) → `delimiter_value_like`
- `vec_field<"d", u32, until<u8{0}>>` → `delimited_buffer_is_byte_wide`
- `until<>` on `basic_field` → `fixed_size_like` (§1.4)
- `until<>` on `fixed_array_field` and on `c_str_field` → the existing
  per-element option classifier

**One constraint on the compile-time tier that is not obvious and will bite.**
The `delimiter_not_found` case at the *default* bound reads 16 MiB one byte at a
time. Under constant evaluation that is roughly 16.8 million loop iterations,
each with a `bit_cast`, a `push_back` and a compare — well past gcc's default
`-fconstexpr-ops-limit` of 33,554,432 and past `-fconstexpr-loop-limit` too. The
`ut` suites must therefore declare a small `max_bytes` for every bound-related
case, and the default-bound path must be exercised only in the GoogleTest suites.
The spec anticipates this in one line ("in the constexpr path a per-byte loop
spends compiler evaluation steps, which are budgeted"); it needs to be an
instruction in 062, not a note.

---

## 9. Documentation (068)

`docs/schema/size-axis.md` is the page whose framing changes, not just its table.
Its opening — "A size is a **value** … written directly as one of five forms" —
and the axis's premise that a size answers "how many bytes does this field
occupy" *before* the read are both now false in one case. The page needs a
paragraph that says the axis has two kinds of member, not a sixth table row
appended silently.

Content required beyond the table rows, each because a reader has no reason to
expect it:

- the write-side rejection (066), which is the only content rule any size form
  imposes
- the bound's changed role: termination condition rather than allocation guard,
  and counting the value rather than the wire, so `max_bytes<79>` admits 79 bytes
  plus a delimiter
- the per-byte cost, stated plainly, with the `read_until` note so the reasoning
  survives the spec being archived — `docs/` is where a reader looks and
  `dev/specs/` is not
- the three read outcomes and the one write outcome, together, in
  `docs/errors.md` alongside the enum block, which currently lists five
  enumerators and will list seven

`docs/schema/index.md`'s descriptor table gains a "Bytes consumed" entry for the
delimited form of `str_field` and `vec_field`. The ELF/Git-shaped example
(a bare NUL-terminated header, `max_bytes` the only bound) goes under
`test/doc_examples/` with its `<!-- docs: -->` binding and `// docs-begin` /
`// docs-end` region, reads through a binary `std::ifstream`, and defines its own
`u8` — the library does not provide that alias, and the spec's snippets assume it.

`README.md`'s roadmap line for read-until: tick it only for what this feature
delivers, and split the line if the sentinel-sequence feature also claims it.

---

## 10. Findings, contradictions and risks

**F1 — 062 is larger than it reads, and two of the additions are hard errors
rather than missing features.** Neither the spec's "Where it lands" paragraph nor
062's criteria mention `field_list_metadata.hpp` or `field_traits.hpp`'s
`field_like`. `extract_length_dependencies` has no primary definition, so the
first schema containing an `until<>` field fails to compile at
`struct_field_list` — before any reader runs, and with a diagnostic pointing at
metadata extraction rather than at anything the author wrote. `field_like`'s
disjunction has the same effect one layer up. Both are one specialization each
(§1.5) and both belong in 062; without them 062 cannot be demonstrated at all.

**F2 — `size_option_like` must widen, and no issue says so.** 062 names
`field_descriptors.hpp:82` as the clause that widens. That clause is never
reached: the option pack classifies entries first, and `until<u8{0}>` would be an
unrecognised entry. One arm in `field_options.hpp` (§1.4).

**F3 — 064's `basic_field` rejection needs production code, which 064's framing
does not anticipate.** 064 says the widened clauses "must not have widened so far
that these compile" — as if the rejection were a property to verify. It is not:
F2's widening is what makes `until<>` reach `basic_field` at all, and the
resulting failure is an incomplete-type arithmetic diagnostic. One clause on
`basic_field` fixes it (§1.4). Conversely, 064's `fixed_array_field` and
`c_str_field` rejections need no code whatsoever — those descriptors take
constraint options only. **064 is therefore one line plus four test targets**,
and its cost is in the control-build verification, not in implementation.

**F4 — 064 asks for diagnostics the repo's own convention says not to pin.**
"Each diagnostic names what was wrong, not merely that a constraint was
unsatisfied" reads as a request for `static_assert` text. But
`must_not_compile/byte_order_misuse.cpp`'s header states the opposite policy —
concept wording differs across the three supported compilers, so pinning text is
non-portable — and `field_options.hpp` states that `static_assert` is reserved
for what a concept *cannot* express (counting). Every rejection here is
expressible as a concept. This design resolves it by making the **concept names**
the diagnostics (`delimiter_value_like`, `delimited_buffer_is_byte_wide`,
`fixed_size_like`) and registering with `add_rejected_case` rather than
`add_rejected_case_matching`. If the user wants sentence-shaped diagnostics here,
that is a deliberate departure and `add_rejected_case_matching` exists for it —
but it should be decided, not drifted into.

**F5 — 067 is a test slice, confirmed by reading rather than assumed.** §5. The
issue offers both outcomes; the answer is the cheap one. Its acceptance criterion
"If `compute_impl` needed changing, no existing use changes behaviour" is
vacuous, and its real content is the doc example and the round trip.

**F6 — the compile-time tier cannot exercise the default bound.** §8's last
paragraph. This is a genuine limit of the per-byte decision under constant
evaluation, not a test-writing preference, and 062's "covered in both forms"
criterion needs reading with it in mind: the `delimiter_not_found` path is
covered in both forms *at a small declared bound*, and at the default bound in
the GoogleTest suite only.

**F7 — the empty-value write touches `const_byte_addressof` on an empty
container.** `write_native` on an empty `std::vector` reaches
`reinterpret_cast<const char*>(obj.data())`, which may be null, and passes it to
`ostream::write(p, 0)`. The hazard predates this feature — a zero-length
`len_from_field` buffer does the same — but 065 makes the empty value a
*required* test case, so it goes from unlikely to certain. §3.1 skips the body
write when the value is empty. Worth noting that the general fix belongs in
`write_native`, not here; this design takes the local one because widening the
fix would change behaviour for every variable-sized write in a slice that is
about delimiters.

**F8 — the spec's PNG snippet omits a bound.** The spec writes
`str_field<"keyword", until<u8{0}>>` while 067 requires "a keyword at PNG's
stated 79-byte maximum … declared as `max_bytes<79>`". Not a contradiction — the
bound is optional and the snippet is illustrative — but the doc example should
carry `max_bytes<79>`, since the whole point of the value-not-wire accounting is
that a schema author transcribes PNG's stated range unadjusted.

**F9 — no issue covers `until<>` behind `maybe`, and it works.** §7, Extension
points. It is not a risk so much as an untested capability; a user will find it.
Listing it in 068's prose or adding one `maybe` case to 062's suite would close
it cheaply.

**Nothing in the spec was found to be wrong.** The two open questions it hands to
the design are settled in §3.2 and §4; its third (naming) is left to the standing
naming sweep as it directs.
