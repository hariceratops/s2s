#ifndef _CAST_HPP_
#define _CAST_HPP_

#include <print>

#include <expected>
#include "field_reader.hpp"
// #include "field_meta.hpp"
#include "field_list.hpp"


// forward declaration
template <field_list_like T>
constexpr auto struct_cast(std::ifstream&) -> std::expected<T, error_code>;

template <typename T>
struct struct_cast_impl;

// todo possible dead code
template <typename... expected_types>
auto is_any_error(const expected_types&... expected_list) {
  return (expected_list && ...);
}

// todo constraints
// todo fix the copying by using reference currently error thrown due to 
// non const lvalue being bound to rvalue
auto operator|(const auto& callable_left, const auto& callable_right) 
  -> std::expected<void, error_code> 
{
  auto res = callable_left();
  return res ? res.error() : callable_right;
}

template <typename field_head, typename... field_rest>
struct struct_cast_impl<struct_field_list<field_head, field_rest...>> {
  using S = struct_field_list<field_head, field_rest...>;
  using R = std::expected<S, error_code>;

  constexpr auto operator()(std::ifstream& ifs) -> R {
    S field_list;
    auto res = (
      read<field_head, S>(static_cast<field_head&>(field_list), field_list, ifs) | 
      ... |
      read<field_rest, S>(static_cast<field_rest&>(field_list), field_list, ifs)
    );
    return res ? field_list : std::unexpected(res.error());
  }
};

template <field_list_like T>
constexpr auto struct_cast(std::ifstream& ifs) -> std::expected<T, error_code> {
  return struct_cast_impl<T>{}(ifs);
}

#endif // _CAST_HPP_
