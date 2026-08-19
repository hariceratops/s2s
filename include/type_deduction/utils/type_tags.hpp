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
//
// A tag's pack carries the narrowest option concept its kind admits — the same
// concept its mirror descriptor alias in field_descriptors.hpp carries. That is
// the whole of how a tag admits a different option set per kind while keeping
// the per-element placeholder idiom: no new mechanism, the same concept on the
// same kind of parameter one layer up. The four tags whose size is fixed by the
// tag itself admit a constraint and nothing else; the three whose extent comes
// off the wire admit a bound as well.
//
// "A bound needs a variable size" is hoisted into which tag accepts one, the
// same way 047 hoisted it into which descriptor accepts one. The three
// container tags require a variable_size_like size, so a fixed size is
// unspellable inside them and accepting a bound is unconditionally right; the
// other five have their size fixed by the tag, so a bound there is
// unconditionally wrong and fails the per-element placeholder constraint as an
// entry that classifies as nothing the tag admits.
//
// Every constraint resolves against the tag's `type` — std::vector<T> for
// as_vec, not T — which is also the type constraint_checker is invoked on, so
// the three agree by construction rather than by convention.

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
template <trivial T, field_option_like<T> auto... opts>
  requires fixed_size_like<size_type_of<size_of_pack<T, opts...>>> &&
           field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
struct as_trivial {
  using type = T;
  static constexpr auto size = size_of_pack<T, opts...>;
  static constexpr auto constraint = constraint_of_pack<T, opts...>;
  static constexpr auto bound = use_default_bound;
};

template <field_list_like T, constraint_option_like<T> auto... opts>
struct as_struct {
  using type = T;
  static constexpr auto size = size_dont_care;
  static constexpr auto constraint = constraint_of_pack<T, opts...>;
  static constexpr auto bound = use_default_bound;
};

// todo how to handle array of array
template <trivial T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
struct as_fixed_arr {
  using type = std::array<T, N>;
  static constexpr auto size = byte_count{N * sizeof(T)};
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = use_default_bound;
};

template <std::size_t N, constraint_option_like<fixed_string<N>> auto... opts>
struct as_fixed_string {
  using type = fixed_string<N>;
  static constexpr auto size = byte_count{N + 1};
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = use_default_bound;
};

// The requires-clause spells std::vector<T> rather than `type`: the clause is
// part of the template head, where the member alias does not exist yet.
template <trivial T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
struct as_vec {
  using type = std::vector<T>;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = bound_of_pack<type, opts...>;
};

template <boundable_field_option_like<std::string> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::string, opts...>>>
struct as_string {
  using type = std::string;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = bound_of_pack<type, opts...>;
};

template <field_list_like T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
struct as_arr_of_records {
  using type = std::array<T, N>;
  static constexpr auto size = size_dont_care;
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = use_default_bound;
};

template <field_list_like T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
struct as_vec_of_records {
  using type = std::vector<T>;
  static constexpr auto size = size_of_pack<type, opts...>;
  static constexpr auto constraint = constraint_of_pack<type, opts...>;
  static constexpr auto bound = bound_of_pack<type, opts...>;
};

template <typename T>
struct is_type_tag;

template <typename T, auto... opts>
struct is_type_tag<as_trivial<T, opts...>> {
  static constexpr bool res = true;
};

template <typename T, std::size_t size, auto... opts>
struct is_type_tag<as_fixed_arr<T, size, opts...>> {
  static constexpr bool res = true;
};

template <std::size_t size, auto... opts>
struct is_type_tag<as_fixed_string<size, opts...>> {
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

template <typename T, auto... opts>
struct is_type_tag<as_struct<T, opts...>> {
  static constexpr bool res = true;
};

// These two had no specialization, so type_tag_like was false for them and
// match_case/branch rejected them outright: both tags are documented but have
// never been usable, and nothing in test/ covered them. Added here because 048
// rewrites this block anyway, and because 051 cannot test that
// as_vec_of_records admits a bound while the tag cannot be named at all.
template <typename T, std::size_t size, auto... opts>
struct is_type_tag<as_arr_of_records<T, size, opts...>> {
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
