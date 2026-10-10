#ifndef _SENTINEL_TERMINATOR_HPP_
#define _SENTINEL_TERMINATOR_HPP_


#include <concepts>
#include <type_traits>

#include "../field/field_accessor.hpp"
#include "../field/field_metafunctions.hpp"
#include "../field/field_traits.hpp"
#include "../field_size/field_size.hpp"
#include "../lib/s2s_traits/type_traits.hpp"
#include "field_list.hpp"
#include "field_list_metadata.hpp"


namespace s2s {
// Cast the sentinel to the named field's type once, rather than promoting the
// field's value up: a signed char field holding 0xff compared against 0xff
// promotes to -1 == 255 and would never match the value that ends the run.
template <auto size, typename V>
constexpr auto equals_sentinel(const V& v) -> bool {
  return v == static_cast<V>(size_type_of<size>::sentinel_value);
}
// On an element the reader produced, the stored value is the wire value.
template <auto size, field_list_like record>
constexpr auto matches_sentinel(const record& r) -> bool {
  using sz = size_type_of<size>;
  return equals_sentinel<size>(field_value_of<field_accessor<sz::sentinel_field>>(r));
}
// Everything else in the element is whatever default construction leaves, which
// the soundness check established is the minimal terminator.
template <auto size, field_list_like record>
constexpr auto synthesised_terminator() -> record {
  using sz = size_type_of<size>;
  record r{};
  auto& v = field_value_of<field_accessor<sz::sentinel_field>>(r);
  v = static_cast<std::remove_cvref_t<decltype(v)>>(sz::sentinel_value);
  return r;
}
// Whether a field emits a determined number of bytes in a default-constructed
// element, which is what the synthesised terminator is. The purpose is the
// determined width; "driven by the named field" is not it, because every
// length-prefixed container is empty in a default-constructed element whichever
// field drives it. A field kind added later reaches the static_assert rather
// than a silent answer.
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
    return true;
  else if constexpr(delimited_field_like<T>)
    return true;
  else if constexpr(variable_sized_field_like<T>)
    // A computed size is a callable over values this check does not have, and
    // on write it is verified against the container rather than derived from
    // it, so one returning non-zero for the default element fails the write.
    return !is_computed_size_v<size_type_of<T::field_size>>;
  else if constexpr(struct_field_like<T>)
    return list_writes_determined_bytes<extract_type_from_field_v<T>>::res;
  else if constexpr(array_of_record_field_like<T>)
    return list_writes_determined_bytes<extract_type_from_array_v<typename T::field_type>>::res;
  else if constexpr(record_sequence_field_like<T>)
    // A counted run is empty. A nested sentinel-terminated run emits its own
    // terminator, whose soundness its own declaration already established.
    return true;
  else if constexpr(optional_field_like<T> || union_field_like<T> ||
                    order_announcing_field_like<T>)
    // Each needs a value this check does not have: a presence predicate over
    // siblings, a held alternative against a discriminant, a byte order
    // resolved from bytes.
    return false;
  else
    static_assert(dependent_false<T>,
      "whether this field kind writes determined bytes in a synthesised "
      "sentinel terminator is unknown to the soundness check");
}

// A per-field entity rather than a fold in a requires-clause: a static_assert
// message cannot interpolate the field's name (C++23), but the failing
// instantiation's template argument list names it.
template <typename f>
struct terminator_field_check {
  static constexpr bool res = writes_determined_bytes<f>();
  static_assert(res,
    "this field of the terminating element does not write a determined number "
    "of bytes when the element is default-constructed, so the synthesised "
    "terminator would not be the minimal sentinel the format expects. Every "
    "field of the element must be fixed-width, a length-prefixed container "
    "(which is empty in the terminator), a delimited field, or a nested record "
    "of such - not a computed size, an optional, or a union");
};

// One requirement, two users: the read test casts the sentinel down to the
// field, and the synthesis assigns it.
template <typename named_type, auto v>
concept sentinel_fits_named_field =
  requires { static_cast<named_type>(v); } && std::equality_comparable<named_type>;

template <typename record, fixed_string sentinel_field, auto sentinel>
struct terminator_synthesis_check;

template <auto metadata, typename... fields, fixed_string S, auto v>
struct terminator_synthesis_check<struct_field_list_impl<metadata, fields...>, S, v> {
  using named = meta::type_of<lookup_field<metadata>(as_sv(S))->id>;
  using named_type = typename named::field_type;

  static constexpr bool named_is_fixed_width = fixed_sized_field_like<named>;
  static_assert(named_is_fixed_width,
    "the field named by until_field_equals must be fixed-width, so the "
    "terminating element has a determined width");

  static constexpr bool sentinel_fits = sentinel_fits_named_field<named_type, v>;
  static_assert(sentinel_fits,
    "the sentinel given to until_field_equals cannot be held by, or compared "
    "against, the named field");

  // Every element would carry the pinned value: either none could ever be a
  // non-terminator, or the terminator could never be written.
  static constexpr bool named_not_frozen = !frozen_field_like<named>;
  static_assert(named_not_frozen,
    "the field named by until_field_equals is pinned to one value by eq, so "
    "the terminating element could not carry the sentinel");

  static constexpr bool sentinel_satisfies_constraint = [] {
    if constexpr(sentinel_fits)
      return named::constraint_checker(static_cast<named_type>(v));
    else
      return false;
  }();
  static_assert(sentinel_satisfies_constraint,
    "the sentinel given to until_field_equals fails the named field's own "
    "constraint, so writing the terminator would fail validation");

  // The write path derives a length target from the contents of the field it
  // sizes and discards the stored value. The terminator's containers are
  // empty, so the byte written is zero.
  static constexpr bool sentinel_survives_derivation =
    !is_length_derived_field<metadata>(as_sv(S)) ||
    (sentinel_fits && static_cast<named_type>(v) == named_type{});
  static_assert(sentinel_survives_derivation,
    "the named field is the declared length of another field in this element, "
    "so the write path derives its value from that field's contents rather than "
    "from the stored one. The synthesised terminator's containers are empty, so "
    "the value written is zero - a non-zero sentinel would never reach the wire "
    "and a run written this way would never terminate on read");

  static constexpr bool fields_determined =
    (terminator_field_check<fields>::res && ...);

  static constexpr bool res = named_is_fixed_width && sentinel_fits &&
                              named_not_frozen && sentinel_satisfies_constraint &&
                              sentinel_survives_derivation && fields_determined;
};

// Repeats announcing_record.hpp's field_table_of pattern-match rather than
// reusing it, which would tie this header to the announcement machinery.
template <typename T>
struct metadata_of;

template <auto metadata, typename... fields>
struct metadata_of<struct_field_list_impl<metadata, fields...>> {
  static constexpr auto value = metadata;
};

// Conjoined in this order on vector_of_records: the synthesis check opens by
// resolving the named field and is ill-formed when the lookup found nothing,
// and conjunction short-circuits during constraint checking.
template <typename S, typename record>
concept sentinel_field_exists =
  !sentinel_terminated_size_like<S> ||
  lookup_field<metadata_of<record>::value>(as_sv(S::sentinel_field)).has_value;

template <typename S, typename record>
concept sentinel_terminator_is_synthesisable =
  !sentinel_terminated_size_like<S> ||
  terminator_synthesis_check<record, S::sentinel_field, S::sentinel_value>::res;
} /* namespace s2s */

#endif // _SENTINEL_TERMINATOR_HPP_
