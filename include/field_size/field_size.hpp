#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_


#include <cstddef>
#include <type_traits>

#include "../field/field_accessor.hpp"
#include "../lib/metaprog/fixed_string_list.hpp"


namespace s2s {
// A size is a value of an empty structural type, and every classification
// trait keys on that value's *type*. The reason is not style: a class-type
// NTTP is a non-deduced context in a partial specialization, so
// `field<id, T, field_accessor<len_source>{}, c>` cannot deduce `len_source`.
// Anything a trait needs to match on therefore has to live in the type.
//
// byte_count is the one exception — the width lives in the value, because no
// trait matches on a width. Only deduce_field_size reads it, and it reads the
// value. That exception is what buys a plain non-template operator""_B.
struct byte_count {
  std::size_t count{};
};

struct size_dont_care_t {};

// Safety is on by default: a field that declares nothing still gets a ceiling.
// The macro is the one global knob — raise it, or set it to SIZE_MAX to turn
// the defaults off wholesale. It cannot reach a declared max_bytes, which is
// the schema author's intent rather than the library's guess.
//
// 16 MiB: far above what fields in real binary formats carry, far below what a
// corrupt u32 can claim, and survivable as an accidental allocation.
#ifndef S2S_DEFAULT_MAX_BYTES
#define S2S_DEFAULT_MAX_BYTES (16u * 1024u * 1024u)
#endif

inline constexpr std::size_t default_max_bytes = S2S_DEFAULT_MAX_BYTES;

template <std::size_t N>
struct max_byte_count_t {
  static constexpr std::size_t count = N;
};

// "no bound declared" is its own type rather than max_byte_count_t<SIZE_MAX>,
// so the default stays a deferral to whatever the macro says at include time
// rather than a number baked into every field.
struct use_default_bound_t {};

template <auto callable, field_name_list req_fields>
struct size_from_fields_t {
  static constexpr auto f = callable;
  static constexpr auto req_field_list = req_fields{};
};

// todo size type for holding multiple sizes in case of union fields
template <auto... sizes>
struct size_choices_t {
  static constexpr auto num_of_choices = sizeof...(sizes);
};

// The spellings a schema writes.
template <fixed_string id>
inline constexpr auto len_from_field = field_accessor<id>{};

// todo constraint for callable
template <auto callable, fixed_string... ids>
inline constexpr auto size_from_fields = size_from_fields_t<callable, field_names_of<ids...>>{};

template <auto callable, fixed_string... ids>
inline constexpr auto len_from_fields = size_from_fields<callable, ids...>;

inline constexpr auto size_dont_care = size_dont_care_t{};

template <std::size_t N>
inline constexpr auto max_bytes = max_byte_count_t<N>{};

inline constexpr auto use_default_bound = use_default_bound_t{};

template <auto... sizes>
inline constexpr auto size_choices = size_choices_t<sizes...>{};

// Identity on an already-value size: 4_B is byte_count{4}.
template <auto size>
inline constexpr auto field_size = size;

// decltype of an auto NTTP parameter is unqualified, but decltype of the
// variable templates above is const-qualified. Strip in one place.
template <auto size>
using size_type_of = std::remove_cvref_t<decltype(size)>;

// Recovering the parameters a trait matched on, out of the size value's type.
template <typename S>
struct len_source_of;

template <fixed_string id>
struct len_source_of<field_accessor<id>> {
  static constexpr auto value = id;
};

template <typename T>
struct is_bound_option {
  static constexpr bool res = false;
};

template <std::size_t N>
struct is_bound_option<max_byte_count_t<N>> {
  static constexpr bool res = true;
};

template <>
struct is_bound_option<use_default_bound_t> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_bound_option_v = is_bound_option<T>::res;

template <typename T>
concept bound_option_like = is_bound_option_v<T>;

// Resolving a declared bound, or deferring to the macro.
template <typename B>
struct bound_value {
  static constexpr std::size_t value = default_max_bytes;
};

template <std::size_t N>
struct bound_value<max_byte_count_t<N>> {
  static constexpr std::size_t value = N;
};

template <auto bound>
inline constexpr std::size_t bound_in_bytes = bound_value<size_type_of<bound>>::value;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_fixed_size {
  static constexpr bool res = false;
};

template <>
struct is_fixed_size<byte_count> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_fixed_size_v = is_fixed_size<T>::res;

template <typename T>
struct is_variable_size {
  static constexpr bool res = false;
};

template <fixed_string id>
struct is_variable_size<field_accessor<id>> {
  static constexpr bool res = true;
};

template <auto callable, field_name_list ids>
struct is_variable_size<size_from_fields_t<callable, ids>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variable_size_v = is_variable_size<T>::res;

// A size produced by a user callable rather than read from a single field.
// The distinction matters only on the write path: len_from_field can be
// inverted and derived, this cannot, so it can only be verified.
template <typename T>
struct is_computed_size {
  static constexpr bool res = false;
};

template <auto callable, field_name_list ids>
struct is_computed_size<size_from_fields_t<callable, ids>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_computed_size_v = is_computed_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept fixed_size_like = is_fixed_size_v<T>;

template <typename T>
concept variable_size_like = is_variable_size_v<T>;

template <typename T>
concept atomic_size = fixed_size_like<T> || variable_size_like<T>;

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <auto... sizes>
  requires (atomic_size<size_type_of<sizes>> && ...)
struct is_selectable_size<size_choices_t<sizes...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_selectable_size<T>::res;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = fixed_size_like<T>    ||
                       variable_size_like<T> ||
                       selectable_size_like<T>;

template <typename T>
struct is_size_dont_care {
  static constexpr bool res = false;
};

template <>
struct is_size_dont_care<size_dont_care_t> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_size_dont_care_v = is_size_dont_care<T>::res;

template <typename T>
concept size_dont_care_like = is_size_dont_care_v<T>;

} /* namespace s2s */

namespace s2s_literals {
constexpr auto operator""_B(unsigned long long n) -> s2s::byte_count {
  return s2s::byte_count{static_cast<std::size_t>(n)};
}
}

#endif // _FIELD_SIZE_HPP_
