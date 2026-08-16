#ifndef _TYPE_TAGS_
#define _TYPE_TAGS_

#include "../../lib/s2s_traits/type_traits.hpp"
#include "../../field_list/field_list_base.hpp"
#include "../../field_size/field_size.hpp"
#include "../../field_size/comptime_field_size_deduce.hpp"
#include "../../field_validation/field_value_constraints.hpp"
#include "../../field/field_options.hpp"


namespace s2s {
// Every tag exposes the four things to_field_choices needs to build a field.
// `constraint` and `bound` are still fixed: the tags admit size entries only
// until the slices that enforce a constraint and a bound land, because a tag
// that accepted an option it silently dropped would be worse than one that
// rejects it.

// Both conjuncts, in this order, and the first is not redundant: it is what
// makes deduce_field_size<size>{}() inside the second a well-formed expression
// at all. With a variable size resolved out of the pack, deduce_field_size
// names a primary template that is declared and never defined, and resting a
// user-facing diagnostic on SFINAE through an incomplete type is not worth the
// line it saves.
//
// as_trivial<u32> now compiles, defaulting to byte_count{sizeof(u32)} the way
// basic_field<"x", u32> already does — a widening that arrives as a side effect
// of the size becoming a pack entry.
template <trivial T, size_option_like<T> auto... opts>
  requires fixed_size_like<size_type_of<size_of_pack<T, opts...>>> &&
           field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
struct as_trivial {
  using type = T;
  static constexpr auto size = size_of_pack<T, opts...>;
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

template <field_list_like T>
struct as_struct {
  using type = T;
  static constexpr auto size = size_dont_care;
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

// todo how to handle array of array
template <trivial T, std::size_t N>
struct as_fixed_arr {
  using type = std::array<T, N>;
  static constexpr auto size = byte_count{N * sizeof(T)};
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

template <std::size_t N>
struct as_fixed_string {
  using type = fixed_string<N>;
  static constexpr auto size = byte_count{N + 1};
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

// The requires-clause spells std::vector<T> rather than `type`: the clause is
// part of the template head, where the member alias does not exist yet.
template <trivial T, size_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
struct as_vec {
  using type = std::vector<T>;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

template <size_option_like<std::string> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::string, opts...>>>
struct as_string {
  using type = std::string;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

template <field_list_like T, std::size_t N>
struct as_arr_of_records {
  using type = std::array<T, N>;
  static constexpr auto size = size_dont_care;
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

template <field_list_like T, size_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
struct as_vec_of_records {
  using type = std::vector<T>;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = no_constraint<type>{};
  static constexpr auto bound = use_default_bound;
};

template <typename T>
struct is_type_tag;

template <typename T, auto... opts>
struct is_type_tag<as_trivial<T, opts...>> {
  static constexpr bool res = true;
};

template <typename T, std::size_t size>
struct is_type_tag<as_fixed_arr<T, size>> {
  static constexpr bool res = true;
};

template <std::size_t size>
struct is_type_tag<as_fixed_string<size>> {
  static constexpr bool res = true;
};

template <typename T, auto... opts>
struct is_type_tag<as_vec<T, opts...>> {
  static constexpr bool res = true;
};

template <auto... opts>
struct is_type_tag<as_string<opts...>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag<as_struct<T>> {
  static constexpr bool res = true;
};

// These two had no specialization, so type_tag_like was false for them and
// match_case/branch rejected them outright: both tags are documented but have
// never been usable, and nothing in test/ covered them. Added here because 048
// rewrites this block anyway, and because 051 cannot test that
// as_vec_of_records admits a bound while the tag cannot be named at all.
template <typename T, std::size_t size>
struct is_type_tag<as_arr_of_records<T, size>> {
  static constexpr bool res = true;
};

template <typename T, auto... opts>
struct is_type_tag<as_vec_of_records<T, opts...>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_type_tag_v = is_type_tag<T>::res;

template <typename T>
concept type_tag_like = is_type_tag_v<T>;
} /* namespace s2s */

#endif // _TYPE_TAGS_
