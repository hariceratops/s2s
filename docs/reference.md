# Reference

The descriptor aliases as declared in `include/api/field_descriptors.hpp`. The
[descriptor table](schema/index.md#the-descriptors) is the readable form; this is the exact one.

```cpp
template <fixed_string id, trivial T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
  requires field_fits_to_underlying_type<size_type, T>
using basic_field = field<id, T, size_type, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using fixed_array_field = field<id, std::array<T, N>, byte_count{N * sizeof(T)}, constraint_of_pack<...>>;

template <fixed_string id, field_list_like T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using array_of_records = field<id, std::array<T, N>, size_dont_care, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<fixed_string<N>>{}>
using fixed_string_field = field<id, fixed_string<N>, fixed<N + 1>, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<T[N]>{}>
using c_arr_field = field<id, T[N], byte_count{N * sizeof(T)}, constraint_of_pack<...>>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<char[N + 1]>{}>
using c_str_field = field<id, char[N + 1], byte_count{N * sizeof(char) + 1}, constraint_of_pack<...>>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, byte_count{N}, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, fixed<expected.size() + 1>, eq{expected}>;

template <fixed_string id, integral T, fixed_size_like size, auto expected>
using magic_number = field<id, T, size, eq{expected}>;

template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, variable_size_like size, auto constraint_on_value = no_constraint<std::string>{}>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, size_dont_care, no_constraint<T>{}>;

template <no_variance_field_like base_field, typename present_only_if>
  requires is_eval_bool_from_fields_v<present_only_if>
using maybe = maybe_field<base_field, present_only_if>;

template <fixed_string id, type_deduction_like type_deducer,
          constraint_option_like<typename type_deducer::variant> auto... opts>
  requires (has_unique_field_choices(extract_field_choices<type_deducer>::value)) &&
           (has_unique_match_values(extract_match_values<type_deducer>::value))
using variance =
  union_field<id, type_deducer,
              constraint_of_pack<typename type_deducer::variant, opts...>>;

template <fixed_string id, field_list_like T, order_deduction_like guide,
          constraint_option_like<T> auto... opts>
using announces_byte_order =
  order_announcing_field<field<id, T, size_dont_care, constraint_of_pack<T, opts...>>,
                         guide>;
```

## The byte-order guide

The declaration surface of [the byte-order axis](schema/byte-order-axis.md),
mirroring the type-deduction constructs one for one.

```cpp
template <auto v, std::endian order>
struct order_case;

template <evaluates_to_bool eval, std::endian order>
struct order_branch;

template <order_case_like... cases>
  requires (sizeof...(cases) > 0)
struct order_switch;

template <order_branch_like... branches>
  requires (sizeof...(branches) > 0)
struct order_if_else;

// order_from<match_field<id>, order_switch<...>>
// order_from<compute_t<callable, R, names>, order_switch<...>>
// order_from<order_if_else<...>>
template <typename... Args>
struct order_from;
```

## The entry points

```cpp
template <fixed_order_schema T, input_stream_like stream>
[[nodiscard]] auto struct_cast_le(stream& s) -> std::expected<T, cast_error>;

template <fixed_order_schema T, input_stream_like stream>
[[nodiscard]] auto struct_cast_be(stream& s) -> std::expected<T, cast_error>;

template <self_announcing_schema T, input_stream_like stream>
[[nodiscard]] auto struct_cast(stream& s) -> std::expected<T, cast_error>;

template <fixed_order_schema T, output_stream_like stream>
[[nodiscard]] auto stream_cast_le(stream& s, const T& obj) -> cast_result;

template <fixed_order_schema T, output_stream_like stream>
[[nodiscard]] auto stream_cast_be(stream& s, const T& obj) -> cast_result;

template <self_announcing_schema T, output_stream_like stream>
[[nodiscard]] auto stream_cast(stream& s, const T& obj) -> cast_result;

template <fixed_layout_schema S, raw_byte_like B, std::size_t extent>
  requires span_covers_record<extent, S>
[[nodiscard]] constexpr auto struct_view_le(std::span<B, extent> bytes)
    -> std::expected<struct_view<S, std::endian::little, std::remove_const_t<B>>, cast_error>;

template <fixed_layout_schema S, raw_byte_like B, std::size_t extent>
  requires span_covers_record<extent, S>
[[nodiscard]] constexpr auto struct_view_be(std::span<B, extent> bytes)
    -> std::expected<struct_view<S, std::endian::big, std::remove_const_t<B>>, cast_error>;
```

`fixed_order_schema` is a schema declaring no byte-order-announcing record;
`self_announcing_schema` is one declaring exactly one. The two are disjoint, so
which entry point a schema takes is settled by the schema, and taking the wrong
one is a compile error rather than a wrong file.

`struct_view_le` and `struct_view_be` take a [fixed-layout schema](#the-fixed-layout-concept)
instead and are described under [Views](#views). **They carry an unsafe
contract** the other entry points do not: see each entry there.

## Views

The guide is [Views](views.md). **A view is not zero-copy**: every field read is
decoded and returned by value, so it saves the fields that are not touched and
buys little for a caller who reads every field.

### `struct_view_le` and `struct_view_be`

`S` is given explicitly; the element type `B` and the extent are deduced from
the span. `B` is `char`, `unsigned char` or `std::byte`, const or not; `signed
char` is not accepted.

Returns the view, or a `cast_error`. The call is not viable, and does not
compile, when `S` is not a `fixed_layout_schema`, or when the span has a static
extent below `wire_width<S>` (`span_covers_record_v` is the atom that fails).
A dynamic extent below it fails with `buffer_exhaustion`, and `failed_at` names
the first top-level field that does not fit. Every constrained field, magic
fields included, is checked in wire order: the first violation is a
`validation_failure` naming the top-level field containing it.

**Unsafe contract.**

- **Lifetime is the caller's.** Nothing checks that `bytes` outlives the
  returned view, or any sub-view taken from it. Using a view after its bytes are
  gone is undefined behaviour.
- **Bounds are checked once, here**: at compile time for a static extent,
  against `bytes.size()` for a dynamic one. A span built from a wrong pointer or
  size cannot be detected.
- **Constraints are checked once, here.** Bytes changed afterwards are read as
  they now are, unvalidated.

### `struct_view<S, order, B>::operator[]`

```cpp
template <typename accessor, auto res = lookup_field<metadata>(as_sv(accessor::field_id))>
  requires (res.has_value)
constexpr auto operator[](accessor) const -> const view_read_t<meta::type_of<res->id>, order, B>;
```

Returns the named field by `const` value, decoded from the bytes at its
constant offset; no other field is read. The return type by field kind:

| Field | Returns |
|---|---|
| `basic_field`, `magic_number` | `T` |
| `fixed_array_field`, `magic_byte_array` | `std::array<T, N>` |
| `fixed_string_field`, `magic_string` | `fixed_string<N>` |
| `c_arr_field` | `std::array<T, N>` |
| `c_str_field` | `std::array<char, N + 1>` |
| `struct_field` | `struct_view<Nested, order, B>`, a sub-view; nothing is decoded |
| `array_of_records` | `array_of_records_view<Elem, N, order, B>` |

`order` is a `std::endian`, so a view's type means the same on every host. A
sub-view is the same template as a top-level view.

**Unsafe contract.** The view, and every sub-view and array view taken from it,
points into the caller's bytes. Reading through one after those bytes are gone
is undefined behaviour, and nothing detects it. The field is decoded from the
bytes as they are now: a constraint checked at creation is not checked again.
No read can fail, so there is no error to return.

### `array_of_records_view<S, N, order, B>`

```cpp
static constexpr auto size() -> std::size_t;                            // N
constexpr auto operator[](std::size_t i) const -> const struct_view<S, order, B>;
```

**Unsafe contract.** `operator[]` is unchecked. `i >= N` is a precondition
violation: it forms an address past the array, and because trailing bytes after
the record are permitted, it can silently read garbage instead of failing.
Compare `i` with `size()` first. During constant evaluation an index leaving the
underlying array is a compile error. The result is a sub-view, so the lifetime
condition above applies to it. There are no iterators.

### `wire_width<S>`

```cpp
template <fixed_layout_schema S>
inline constexpr std::size_t wire_width = record_layout<S>::width;
```

The record's width on the wire in bytes, summed over the fields' declared
sizes, nested records and arrays of records included. It is not `sizeof`. It
needs no view, which is what stepping through a file needs.

It describes the schema only; whether any given span has that many bytes is
something only the caller knows. **Unsafe contract:** a stride computed from it
over a span the caller got wrong is not detected until creation, and not at all
if the wrong pointer is within the span.

### The fixed-layout concept

```cpp
template <typename S>
concept fixed_layout_schema =
  field_list_like<S> &&
  has_no_announcing_record<S> &&
  has_no_optional_field<S> &&
  has_no_variant_field<S> &&
  every_field_has_compile_time_width<S>;
```

It is pure: it evaluates to `false` rather than failing a `static_assert`, so
it can be tested both ways. The conjuncts are ordered most-specific first, and
the width rule, which every rejected kind fails, last, so that a rejected
schema is reported under the name that describes it. Each sub-concept is a
single variable-template atom; **the atom is the name a compiler prints**,
which is why it is listed.

| Sub-concept | Atom in the diagnostic | Rejects, and why |
|---|---|---|
| `has_no_announcing_record` | `has_no_announcing_record_v` | `announces_byte_order` at any depth: the order is data, and a view's order is part of its type |
| `has_no_optional_field` | `has_no_optional_field_v` | every `maybe`, at any depth through `struct_field` and `array_of_records`: presence depends on the data and moves every later offset |
| `has_no_variant_field` | `has_no_variant_field_v` | every `variance`, including one whose alternatives share a width: the rule is about the schema |
| `every_field_has_compile_time_width` | `every_field_has_compile_time_width_v` | `vec_field`, `str_field` and `vector_of_records`, counted or terminated by a sentinel: their length is read from the wire |

`span_covers_record<extent, S>` (atom `span_covers_record_v`) is the static-extent
check: true for a dynamic extent, and otherwise when `extent >= wire_width<S>`.
`raw_byte_like<B>` accepts `char`, `unsigned char` and `std::byte`, const or
not.

## Known limitations

Recorded here because they constrain what a schema can express, not because
they are planned work:

- **Vector lengths are element counts, not byte counts.** A `vec_field`'s size
  axis yields how many elements to read. There is no overload taking a length in
  bytes, so a format that prefixes a byte count for a vector of multi-byte
  elements needs a `len_from_fields` callable to do the division.
- **The vector descriptors are fixed to `std::vector`.** There is no hook for a
  custom container or allocator, so a schema cannot read into a
  `pmr::vector` or a fixed-capacity vector.
- **The string descriptors are fixed to `char`.** `str_field` is `std::string`
  and `fixed_string` holds `char`; wide and UTF-16 character types are not
  supported.
- **`size_choices` cannot be declared** — see
  [The size axis](schema/size-axis.md#size_choices-is-not-currently-declarable).
- **`range`, `is_in_open_range` and `is_in_closed_range` do not compile** — see
  [Constraints](constraints.md#range-constraints-do-not-currently-compile).
- **A schema may declare at most one byte-order-announcing record.** Two
  anywhere in a schema is a compile error. A second one taking over from the
  first follows from the same stream-position rule at no extra engine cost; it
  is left undecided until a real format settles what it should mean.
- **A resolved byte order always governs the rest of the file.** An announcement
  cannot be scoped to its own subtree, which is what an embedded format carrying
  its own marker needs — Exif MakerNotes is the standing example. Reading such a
  region as a separate cast is the workaround.
- **Only the announcing record's own fields are checked for byte-order
  agnosticism.** A field placed as an earlier sibling of the announcing record
  is read before the order is known and is not diagnosed — see
  [The byte-order axis](schema/byte-order-axis.md#what-an-announcing-record-may-contain).

The last three are current limitations rather than design principles, and
lifting any of them is additive.
