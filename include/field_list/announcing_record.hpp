#ifndef _ANNOUNCING_RECORD_HPP_
#define _ANNOUNCING_RECORD_HPP_


#include "../field/field_metafunctions.hpp"
#include "../field/field_traits.hpp"
#include "../order_deduction/order_deduction.hpp"
#include "field_list.hpp"
#include "field_list_metadata.hpp"


namespace s2s {
// The names a byte-order announcement reads. Extracted with the same shape and
// the same helpers extract_type_deduction_dependencies uses, but deliberately
// not through it: that metafunction feeds a dependency table consumed by
// is_dependencies_resolved, which looks every name up in the *declaring* list's
// field table and dereferences the result. An announcement's names are fields
// of the inner record, so routing them there would make every such schema a
// constant-evaluation error.
//
// Existence is also the whole obligation. The ordering half that check performs
// is vacuous on this axis: every field of the announcing record is read before
// resolution runs, so no name can be "not yet read".
//
// Sits above field_list.hpp rather than inside the metadata, because reaching
// the inner record's table means pattern-matching struct_field_list_impl, which
// the metadata header is included by.
template <typename guide>
struct order_deduction_dependencies;

template <fixed_string id, typename _switch>
struct order_deduction_dependencies<order_from<match_field<id>, _switch>> {
  static constexpr auto value = dep_vec(as_sv(id));
};

template <auto callable, typename R, fixed_string... req_fields, typename _switch>
struct order_deduction_dependencies<
  order_from<compute_t<callable, R, fixed_string_list<req_fields...>>, _switch>
>
{
  static constexpr auto value = dep_vec(as_sv(req_fields)...);
};

template <typename... branches>
struct order_deduction_dependencies<order_from<order_if_else<branches...>>> {
  static constexpr dep_vec deps[64] = {dep_vec(extract_req_fields_from_clause_v<branches>)...};
  static constexpr auto flat_values = flatten(deps);
  static constexpr auto value = remove_duplicates(flat_values);
};


template <typename T>
struct field_table_of;

template <auto metadata, typename... fields>
struct field_table_of<struct_field_list_impl<metadata, fields...>> {
  static constexpr auto value = meta::type_of<metadata>::field_table;
};


constexpr auto all_names_present(const field_table_t& table, const dep_vec& names) -> bool {
  for(auto name: names) {
    if(!table[name].has_value())
      return false;
  }
  return true;
}


template <typename T>
struct announcement_names_resolve {
  static constexpr bool res = true;
};

template <fixed_string id, field_list_like T, auto size, auto constraint, auto bound,
          typename guide>
struct announcement_names_resolve<
  order_announcing_field<field<id, T, size, constraint, bound>, guide>
>
{
  static constexpr bool res =
    all_names_present(field_table_of<T>::value, order_deduction_dependencies<guide>::value);
};

template <typename T>
inline constexpr bool announcement_names_resolve_v = announcement_names_resolve<T>::res;

// A field is order-agnostic when reading it yields the same value whichever
// state the order cell is in. Every field of an announcing record is read
// before its own deduction runs, so this predicate is what makes the seed the
// entry point supplies unobservable — and what forces a marker to be spelled as
// bytes rather than as a multi-byte integer.
//
// A variable-length field's length lives in a different field of the same
// record and is judged on its own, which is the right answer: a str_field whose
// length comes from a u16 is agnostic in its payload, and that u16 is not, so
// the record is rejected — correctly, since the u16 is read before the order is
// known.
template <typename T>
constexpr auto is_order_agnostic_field() -> bool;

template <typename T>
struct is_order_agnostic_list;

template <auto metadata, typename... fields>
struct is_order_agnostic_list<struct_field_list_impl<metadata, fields...>> {
  static constexpr bool res = (is_order_agnostic_field<fields>() && ...);
};

template <typename T>
inline constexpr bool is_order_agnostic_list_v = is_order_agnostic_list<T>::res;

template <typename T>
constexpr auto is_order_agnostic_value() -> bool {
  if constexpr(trivial<T>)
    return sizeof(T) == 1;
  // byteswap_elements returns immediately on a fixed_string, and the write path
  // routes it to write_native, so it reads the same either way. A std::string is
  // chars.
  else if constexpr(fixed_string_like<T> || string_like<T>)
    return true;
  else if constexpr(fixed_array_like<T>)
    return is_order_agnostic_value<extract_type_from_array_v<T>>();
  else if constexpr(is_c_array_v<T>)
    return is_order_agnostic_value<std::remove_extent_t<T>>();
  else if constexpr(vector_like<T>)
    return is_order_agnostic_value<extract_type_from_vec_t<T>>();
  else if constexpr(field_list_like<T>)
    return is_order_agnostic_list_v<T>;
  else
    static_assert(dependent_false<T>,
      "the byte-order agnosticism of this field type is unknown to the check "
      "that guards an announcing record's fields");
}


template <typename T>
struct order_agnostic_choices;

template <typename... choices>
struct order_agnostic_choices<field_choice_list<choices...>> {
  static constexpr bool res = (is_order_agnostic_field<choices>() && ...);
};

// The wrapper kinds reduce to what they wrap; everything else reduces to its
// value type. An order_announcing_field needs no arm of its own: its inherited
// field_type is the announced record, so the last branch descends into it.
template <typename T>
constexpr auto is_order_agnostic_field() -> bool {
  if constexpr(struct_field_like<T>)
    return is_order_agnostic_list_v<extract_type_from_field_v<T>>;
  else if constexpr(array_of_record_field_like<T>)
    return is_order_agnostic_list_v<extract_type_from_array_v<typename T::field_type>>;
  else if constexpr(vector_of_record_field_like<T>)
    return is_order_agnostic_list_v<extract_type_from_vec_t<typename T::field_type>>;
  else if constexpr(optional_field_like<T>)
    return is_order_agnostic_field<typename T::field_base_type>();
  else if constexpr(union_field_like<T>)
    return order_agnostic_choices<typename T::field_choices>::res;
  else
    return is_order_agnostic_value<typename T::field_type>();
}


template <typename T>
struct record_of_announcement_is_order_agnostic {
  static constexpr bool res = true;
};

template <fixed_string id, field_list_like T, auto size, auto constraint, auto bound,
          typename guide>
struct record_of_announcement_is_order_agnostic<
  order_announcing_field<field<id, T, size, constraint, bound>, guide>
>
{
  static constexpr bool res = is_order_agnostic_list_v<T>;
};

template <typename T>
inline constexpr bool record_of_announcement_is_order_agnostic_v =
  record_of_announcement_is_order_agnostic<T>::res;


// One depth-first census, three consumers: 056's local checks read it to find
// the announcement a list declares, 057 rejects an off-spine one, and 060
// rejects a second one anywhere. Two counts rather than one, because an
// announcement reached conditionally (a maybe, a union alternative) or
// repeatedly (an array or vector of records) is a different rejection from a
// second one on the spine.
struct announcement_census {
  std::size_t on_spine;
  std::size_t off_spine;
};

constexpr auto operator+(announcement_census a, announcement_census b) -> announcement_census {
  return {a.on_spine + b.on_spine, a.off_spine + b.off_spine};
}

constexpr auto folded_off_spine(announcement_census c) -> announcement_census {
  return {0, c.on_spine + c.off_spine};
}

template <typename T>
constexpr auto census_of_field() -> announcement_census;

template <typename... fields>
struct census_of_fields {
  static constexpr auto value = (announcement_census{0, 0} + ... + census_of_field<fields>());
};

template <typename T>
struct census_of_list;

template <auto metadata, typename... fields>
struct census_of_list<struct_field_list_impl<metadata, fields...>> {
  static constexpr auto value = census_of_fields<fields...>::value;
};

template <typename T>
inline constexpr auto census_of_list_v = census_of_list<T>::value;

template <typename... choices>
struct census_of_choices;

template <typename... choices>
struct census_of_choices<field_choice_list<choices...>> {
  static constexpr auto value = (announcement_census{0, 0} + ... + census_of_field<choices>());
};

template <typename T>
constexpr auto census_of_field() -> announcement_census {
  // Descends into the announced record too, so a second announcement declared
  // inside one is counted rather than hidden by the field that carries it.
  if constexpr(order_announcing_field_like<T>)
    return announcement_census{1, 0} + census_of_list_v<typename T::field_type>;
  else if constexpr(struct_field_like<T>)
    return census_of_list_v<extract_type_from_field_v<T>>;
  else if constexpr(array_of_record_field_like<T>)
    return folded_off_spine(census_of_list_v<extract_type_from_array_v<typename T::field_type>>);
  else if constexpr(vector_of_record_field_like<T>)
    return folded_off_spine(census_of_list_v<extract_type_from_vec_t<typename T::field_type>>);
  else if constexpr(optional_field_like<T>)
    return folded_off_spine(census_of_field<typename T::field_base_type>());
  else if constexpr(union_field_like<T>)
    return folded_off_spine(census_of_choices<typename T::field_choices>::value);
  else
    return announcement_census{0, 0};
}

// Which entry point a schema takes is a property of the schema, so the two
// concepts are the shape of it: one for a format whose order is fixed by the
// format, one for a format whose bytes decide. Each is backed by a struct of
// static_asserts, following dependency_check, so the failure arrives as a
// sentence as well as an unsatisfied constraint.
//
// One pair serves both directions rather than four structs differing only in
// wording — the rule is the same rule, so each message names the read call and
// the write call together.
//
// A count of two or more never reaches either: announcing_record_check rejects
// it at schema declaration, which is strictly earlier and names a better
// problem.
template <typename T>
struct self_announcing_check {
  static constexpr auto count = census_of_list_v<T>.on_spine;

  static_assert(count != 0,
    "struct_cast and stream_cast are for a schema that declares a "
    "byte-order-announcing record — one whose bytes decide the order, like "
    "TIFF's II/MM. This schema declares none, so its byte order is a fixed "
    "fact about the format: call struct_cast_le or struct_cast_be to read it, "
    "or stream_cast_le or stream_cast_be to write it");

  static constexpr bool res = (count == 1);
};

template <typename T>
concept self_announcing_schema = field_list_like<T> && self_announcing_check<T>::res;

template <typename T>
struct fixed_order_check {
  static constexpr auto count = census_of_list_v<T>.on_spine;

  static_assert(count == 0,
    "struct_cast_le, struct_cast_be, stream_cast_le and stream_cast_be are for "
    "a schema whose byte order is fixed by the format. This schema declares a "
    "byte-order-announcing record, so the file decides its own order: call "
    "struct_cast to read it, or stream_cast to write it. There is deliberately "
    "no way to force a fixed order over a self-announcing schema");

  static constexpr bool res = (count == 0);
};

template <typename T>
concept fixed_order_schema = field_list_like<T> && fixed_order_check<T>::res;
} /* namespace s2s */


#endif // _ANNOUNCING_RECORD_HPP_
