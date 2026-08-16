#ifndef _FIELD_OPTIONS_HPP_
#define _FIELD_OPTIONS_HPP_


// comptime_field_size_deduce, not field_size_deduce: the latter needs
// struct_field_list_impl, which would pull field_list.hpp underneath
// type_tags.hpp and close a different loop than the one this header exists to
// break. Only the compile-time size deduction is needed here.
#include "../field_size/field_size.hpp"
#include "../field_size/comptime_field_size_deduce.hpp"
#include "../field_validation/field_value_constraints.hpp"


// Split out of api/field_descriptors.hpp so type_tags.hpp can reach it.
// field_descriptors.hpp includes type_deduction_traits.hpp, which reaches
// type_tags.hpp through switch_traits -> switch -> match_case, so a tag
// including the descriptors header would close a five-header loop. With
// #ifndef guards rather than modules that would not surface as a circular
// include: the second inclusion is silently empty and the error lands as
// "size_of_pack is not a member of s2s" somewhere unrelated.
//
// It sits in field/ rather than api/ because api/ is the user-facing surface
// and type_deduction/ reaching up into it would invert the layering. This is
// machinery about a descriptor's parameters, and field/ is the layer both sit
// above.
namespace s2s {
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

// A bound is meaningful only where wire input drives the allocation, so only
// the three container descriptors admit one. Everywhere else max_bytes fails
// the per-element placeholder constraint exactly as an unrecognised entry
// does — the relationship "a bound needs a variable size" is hoisted into
// *which descriptor* accepts it rather than checked between two pack entries,
// which keeps classification per-element and keeps the diagnostic readable.
template <typename B, typename T>
concept bound_pack_option_like = bound_option_like<B>;

template <typename O, typename T>
concept boundable_field_option_like = size_option_like<O, T>       ||
                                      constraint_option_like<O, T> ||
                                      bound_pack_option_like<O, T>;

// Applied per element rather than as a fold in a requires-clause: a fold names
// the fold and dumps the whole pack, while this isolates the offending entry
// and prints both things it could have been.
template <typename T, auto... opts>
inline constexpr std::size_t size_option_count =
  (0u + ... + (size_option_like<size_type_of<opts>, T> ? 1u : 0u));

template <typename T, auto... opts>
inline constexpr std::size_t constraint_option_count =
  (0u + ... + (constraint_option_like<size_type_of<opts>, T> ? 1u : 0u));

template <typename T, auto... opts>
inline constexpr std::size_t bound_option_count =
  (0u + ... + (bound_pack_option_like<size_type_of<opts>, T> ? 1u : 0u));

// A concept can reject an entry it cannot classify but cannot count how many
// classified the same way, so duplicates are the one sanctioned static_assert.
template <typename T, auto... opts>
struct pack_options {
  static_assert(size_option_count<T, opts...> <= 1,
                "a field takes at most one size option");
  static_assert(constraint_option_count<T, opts...> <= 1,
                "a field takes at most one constraint option");
  static_assert(bound_option_count<T, opts...> <= 1,
                "a field takes at most one max_bytes option");
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

template <typename T, auto... opts>
struct bound_in_pack {
  static constexpr auto value = use_default_bound;
};

template <typename T, auto head, auto... tail>
struct bound_in_pack<T, head, tail...> {
  static constexpr auto value = [] {
    if constexpr(bound_pack_option_like<size_type_of<head>, T>)
      return head;
    else
      return bound_in_pack<T, tail...>::value;
  }();
};

// Order independence is exactly this scan; nothing else is needed. Deriving
// from pack_options is what instantiates it, so the duplicate assertions fire.
template <typename T, auto... opts>
struct resolved_options : pack_options<T, opts...> {
  static constexpr auto size = size_in_pack<T, opts...>::value;
  static constexpr auto constraint = constraint_in_pack<T, opts...>::value;
  static constexpr auto bound = bound_in_pack<T, opts...>::value;
};

template <typename T, auto... opts>
inline constexpr auto size_of_pack = resolved_options<T, opts...>::size;

template <typename T, auto... opts>
inline constexpr auto constraint_of_pack = resolved_options<T, opts...>::constraint;

template <typename T, auto... opts>
inline constexpr auto bound_of_pack = resolved_options<T, opts...>::bound;
} /* namespace s2s */

#endif /* _FIELD_OPTIONS_HPP_ */
