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
```

`fixed_order_schema` is a schema declaring no byte-order-announcing record;
`self_announcing_schema` is one declaring exactly one. The two are disjoint, so
which entry point a schema takes is settled by the schema, and taking the wrong
one is a compile error rather than a wrong file.

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
