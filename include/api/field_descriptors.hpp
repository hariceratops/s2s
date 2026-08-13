#ifndef _FIELD_DESCRIPTORS_HPP_
#define _FIELD_DESCRIPTORS_HPP_


#include "../lib/s2s_traits/type_traits.hpp"
#include "../field/field.hpp"
#include "../field_list/field_list_base.hpp"
#include "../field_size/field_size.hpp"
#include "../field_size/field_size_deduce.hpp"
#include "../field_validation/field_value_constraints.hpp"
#include "../type_deduction/type/type_deduction_traits.hpp"
#include "../type_deduction/type/type_deduction_metafunctions.hpp"


namespace s2s {
struct always_true {
  // const: compute_impl invokes the callable as a const NTTP, so without this
  // always_present fails to compile in either direction.
  constexpr auto operator()() const -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}>;

template <auto size, typename field_type>
concept field_fits_to_underlying_type = deduce_field_size<size>{}() <= sizeof(field_type);

// The trailing options of a descriptor are an unordered pack: a size, a
// constraint, either, or neither. Classification is by the option's *type*,
// which is also why these concepts take it first — a placeholder constraint
// `field_option_like<T> auto... opts` substitutes decltype(opt) as the first
// argument, so a value-parameterised concept could not be used this way at all.
template <typename S, typename T>
concept size_option_like = fixed_size_like<S>    ||
                           variable_size_like<S> ||
                           size_dont_care_like<S> ||
                           selectable_size_like<S>;

template <typename C, typename T>
concept constraint_option_like = requires (const C& c, const T& v) {
  { c(v) } -> std::same_as<bool>;
};

template <typename O, typename T>
concept field_option_like = size_option_like<O, T> || constraint_option_like<O, T>;

// Applied per element rather than as a fold in a requires-clause: a fold names
// the fold and dumps the whole pack, while this isolates the offending entry
// and prints both things it could have been.
template <typename T, auto... opts>
inline constexpr std::size_t size_option_count =
  (0u + ... + (size_option_like<size_type_of<opts>, T> ? 1u : 0u));

template <typename T, auto... opts>
inline constexpr std::size_t constraint_option_count =
  (0u + ... + (constraint_option_like<size_type_of<opts>, T> ? 1u : 0u));

// A concept can reject an entry it cannot classify but cannot count how many
// classified the same way, so duplicates are the one sanctioned static_assert.
template <typename T, auto... opts>
struct pack_options {
  static_assert(size_option_count<T, opts...> <= 1,
                "a field takes at most one size option");
  static_assert(constraint_option_count<T, opts...> <= 1,
                "a field takes at most one constraint option");
};

template <typename T, auto... opts>
struct size_in_pack {
  static constexpr auto value = byte_count{sizeof(T)};
};

template <typename T, auto head, auto... tail>
struct size_in_pack<T, head, tail...> {
  static constexpr auto value = [] {
    if constexpr(size_option_like<size_type_of<head>, T>)
      return head;
    else
      return size_in_pack<T, tail...>::value;
  }();
};

template <typename T, auto... opts>
struct constraint_in_pack {
  static constexpr auto value = no_constraint<T>{};
};

template <typename T, auto head, auto... tail>
struct constraint_in_pack<T, head, tail...> {
  static constexpr auto value = [] {
    if constexpr(constraint_option_like<size_type_of<head>, T>)
      return head;
    else
      return constraint_in_pack<T, tail...>::value;
  }();
};

// Order independence is exactly this scan; nothing else is needed. Deriving
// from pack_options is what instantiates it, so the duplicate assertions fire.
template <typename T, auto... opts>
struct resolved_options : pack_options<T, opts...> {
  static constexpr auto size = size_in_pack<T, opts...>::value;
  static constexpr auto constraint = constraint_in_pack<T, opts...>::value;
};

template <typename T, auto... opts>
inline constexpr auto size_of_pack = resolved_options<T, opts...>::size;

template <typename T, auto... opts>
inline constexpr auto constraint_of_pack = resolved_options<T, opts...>::constraint;

template <fixed_string id, integral T, field_option_like<T> auto... opts>
  requires field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
using basic_field = field<id, T, size_of_pack<T, opts...>, constraint_of_pack<T, opts...>>;

template <fixed_string id, field_containable T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
using fixed_array_field =
  field<id, std::array<T, N>, byte_count{N * sizeof(T)}, constraint_of_pack<std::array<T, N>, opts...>>;

template <fixed_string id, field_list_like T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
using array_of_records =
  field<id, std::array<T, N>, size_dont_care, constraint_of_pack<std::array<T, N>, opts...>>;

template <fixed_string id, std::size_t N, constraint_option_like<fixed_string<N>> auto... opts>
using fixed_string_field =
  field<id, fixed_string<N>, byte_count{N + 1}, constraint_of_pack<fixed_string<N>, opts...>>;

template <fixed_string id, field_containable T, std::size_t N,
          constraint_option_like<T[N]> auto... opts>
using c_arr_field = field<id, T[N], byte_count{N * sizeof(T)}, constraint_of_pack<T[N], opts...>>;

template <fixed_string id, std::size_t N, constraint_option_like<char[N + 1]> auto... opts>
using c_str_field =
  field<id, char[N + 1], byte_count{N * sizeof(char) + 1}, constraint_of_pack<char[N + 1], opts...>>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, byte_count{N}, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, byte_count{expected.size() + 1}, eq{expected}>;

template <fixed_string id, integral T, auto size, auto expected>
  requires fixed_size_like<size_type_of<size>>
using magic_number = field<id, T, size, eq{expected}>;

// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
using vec_field =
  field<id, std::vector<T>, size_of_pack<std::vector<T>, opts...>,
        constraint_of_pack<std::vector<T>, opts...>>;

template <fixed_string id, field_list_like T, field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
using vector_of_records =
  field<id, std::vector<T>, size_of_pack<std::vector<T>, opts...>,
        constraint_of_pack<std::vector<T>, opts...>>;

// todo check if this will work for all char types like wstring
template <fixed_string id, field_option_like<std::string> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::string, opts...>>>
using str_field =
  field<id, std::string, size_of_pack<std::string, opts...>,
        constraint_of_pack<std::string, opts...>>;

template <fixed_string id, field_list_like T, constraint_option_like<T> auto... opts>
using struct_field = field<id, T, size_dont_care, constraint_of_pack<T, opts...>>;

template <no_variance_field_like base_field, typename present_only_if>
  requires is_eval_bool_from_fields_v<present_only_if>
using maybe = maybe_field<base_field, present_only_if>;


template <fixed_string id, type_deduction_like type_deducer>
  requires (has_unique_field_choices(extract_field_choices<type_deducer>::value)) &&
           (has_unique_match_values(extract_match_values<type_deducer>::value))
using variance = union_field<id, type_deducer>;

} /* namespace s2s */

#endif /* _FIELD_DESCRIPTORS_HPP_ */
