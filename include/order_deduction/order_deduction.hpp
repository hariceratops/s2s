#ifndef _ORDER_DEDUCTION_HPP_
#define _ORDER_DEDUCTION_HPP_


#include <bit>

#include "../type_deduction/type/type.hpp"


namespace s2s {
// The schema speaks std::endian, not cast_endianness: `II` means
// little-endian on every machine, while cast_endianness is relative to the
// host and therefore not something a schema author can write. The collapse
// happens once, where the order is resolved.
template <auto v, std::endian order>
struct order_case {
  static constexpr auto value = v;
  static constexpr auto byte_order = order;
};

template <typename T>
struct is_order_case;

template <auto v, std::endian order>
struct is_order_case<order_case<v, order>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_order_case {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_order_case_v = is_order_case<T>::res;

template <typename T>
concept order_case_like = is_order_case_v<T>;


// Deliberately not type_switch parameterised over an order-carrying tag:
// type_switch exposes `variant` as a member typedef, which forces every case's
// type_tag::type to exist, and giving an order outcome a ::type would make
// variance<"x", type<..., type_switch<match_case<1, as_order<...>>>>> a schema
// that compiles into a variant of two std::endians. A spelling that is
// nonsense and accepted is worse than one that does not exist. The evaluation
// logic — the part that is code — is shared; only these carriers are parallel.
template <order_case_like... cases>
  requires (sizeof...(cases) > 0)
struct order_switch {};

template <typename T>
struct is_order_switch;

template <order_case_like... cases>
struct is_order_switch<order_switch<cases...>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_order_switch {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_order_switch_v = is_order_switch<T>::res;

template <typename T>
concept order_switch_like = is_order_switch_v<T>;


// Mirrors `type`: the same input forms feeding the same case list, with a
// byte order as the outcome instead of a variant alternative. Names inside it
// resolve in the announcing record's own field table, one level below the list
// that declares the announcement.
template <typename... Args>
struct order_from;

template <fixed_string id, typename _switch>
struct order_from<match_field<id>, _switch> {};

template <typename T>
struct is_order_deduction;

template <fixed_string id, typename _switch>
struct is_order_deduction<order_from<match_field<id>, _switch>> {
  static constexpr bool res = order_switch_like<_switch>;
};

template <typename T>
struct is_order_deduction {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_order_deduction_v = is_order_deduction<T>::res;

template <typename T>
concept order_deduction_like = is_order_deduction_v<T>;
} /* namespace s2s */


#endif // _ORDER_DEDUCTION_HPP_
