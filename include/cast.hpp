#ifndef _CAST_HPP_
#define _CAST_HPP_

#include <print>

#include <expected>
#include "field_reader.hpp"
// #include "field_meta.hpp"
#include "field_list.hpp"
#include "error.hpp"


// forward declaration
template <field_list_like T>
constexpr auto struct_cast(std::ifstream&) -> std::expected<T, cast_error>;

template <typename T>
struct struct_cast_impl;

template <typename... fields>
struct struct_cast_impl<struct_field_list<fields...>> {
  using S = struct_field_list<fields...>;
  using R = std::expected<S, cast_error>;

  constexpr auto operator()(std::ifstream& ifs) -> R {
    S field_list;
    read_result pipeline_seed{};
    auto res = (
      pipeline_seed |
      ... |
      read_field<fields, S>(static_cast<fields&>(field_list), field_list, ifs)
    );
    return res ? R(field_list) : std::unexpected(res.error());
  }
};

template <field_list_like T>
constexpr auto struct_cast(std::ifstream& ifs) -> std::expected<T, cast_error> {
  return struct_cast_impl<T>{}(ifs);
}

#endif // _CAST_HPP_
