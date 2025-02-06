#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_

#include <cstring>
#include <print>
#include <fstream>
#include <expected>
#include <utility>
#include "field_traits.hpp"
#include "size_deduce.hpp"
#include "error.hpp"


using read_result = std::expected<void, cast_error>;


template<variant_like T, typename F>
constexpr auto operator|(std::expected<T, cast_error>&& exp, F&& func) {
  return exp ? func(exp) : exp;
}


constexpr auto operator|(read_result& lhs, auto functor) {
  if(lhs) return functor(*lhs); else return lhs;
}


template <typename T>
// todo specialise for non scalar type to facilitate endianness specific vector read
constexpr auto raw_read(T& value, std::ifstream& ifs, std::size_t size_to_read) 
  -> read_result 
{
  if(!ifs.read(byte_addressof(value), size_to_read))
    return std::unexpected(cast_error::buffer_exhaustion);
  return {};
}


template <typename T, field_list_like F>
constexpr auto read_scalar(T& value, std::size_t size_to_read, std::ifstream& ifs)
  -> read_result
{
  return raw_read(value, ifs, size_to_read);
}


template <typename T, field_list_like F>
constexpr auto read_vector(T& value, std::size_t len_to_read, std::ifstream& ifs) 
  -> read_result 
{
  constexpr auto size_of_one_elem = sizeof(decltype(value){}[0]);
  return raw_read(value, ifs, len_to_read * size_of_one_elem);
}


// todo ctad read<t,f>
// todo inheritance for ctor boilerplate removal: read<t,f>
// todo take expected<flist, error>|flist& as argument to operator
template <typename F, typename L>
struct read;

template <fixed_sized_field_like T, field_list_like F>
struct read<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read(T& field, std::ifstream& ifs)
    : field(field), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    return read_scalar(field.value, field_list, ifs);
  }
};


template <variable_sized_field_like T, field_list_like F>
struct read<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read(T& field, std::ifstream& ifs)
    : field(field), ifs(ifs) {}

  constexpr auto operator()(F& field_list) const -> read_result {
    using field_size = typename T::field_size;
    constexpr auto len_to_read = deduce_field_size<field_size>{}(field_list);
    return read_vector(field.value, field_list, ifs);
  }
};


template <field_list_like T, field_list_like F>
struct read<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read(T& field, std::ifstream& ifs)
    : field(field), ifs(ifs) {}

  constexpr auto operator()([[maybe_unused]] F& field_list) const -> read_result {
    auto res = struct_cast<T>(ifs);
    if(!res)
      return std::unexpected(res.error());
    // todo move?
    field.value = *res;
    return {};
  }
};


template <optional_field_like T, field_list_like F>
struct read<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read(T& field, std::ifstream& ifs): 
    field(field), ifs(ifs) {}
  
  constexpr auto operator()(F& field_list) -> read_result {
    if(!typename T::field_presence_checker{}(field_list)) {
      field.value = std::nullopt;
      return {};
    }
    using field_base_type_t = typename T::field_base_type;
    constexpr field_base_type_t base_field{};
    constexpr read<field_base_type_t, F> reader(base_field, field_list, ifs);
    constexpr auto res = reader();
    if(!res) 
      return std::unexpected(res.error());
    // todo is move guaranteed
    field.value = base_field.value;
    return {};
  }
};


// Helper function to read bytes into the variant
template<std::size_t idx, typename T, typename F, typename V>
struct read_variant_impl {
  std::ifstream& ifs;
  V& variant;
  std::size_t idx_r;
  F& field_list;

  constexpr explicit read_variant_impl(
    std::ifstream& ifs, 
    V& variant, 
    F& field_list,
    std::size_t idx_r) :
      ifs(ifs), variant(variant), field_list(field_list), idx_r(idx_r) {}

  constexpr auto operator()() -> read_result {
    if (idx_r != idx) 
      return {};

    T field;
    auto reader = read<T, F>(field, field_list, ifs);
    constexpr auto res = reader();
    if(!res)
      return std::unexpected(res.error());
    variant = std::move(field.value);
    return {};
  }
};


template <typename T, typename F, typename field_choices, typename idx_seq>
struct read_variant_helper;

template <typename T, typename F, typename field_head, std::size_t idx_head, typename... fields, std::size_t... idx>
struct read_variant_helper<T, F, field_choice_list<field_head, fields...>, std::index_sequence<idx_head, idx...>> {
  T& field;
  F& field_list;
  std::ifstream& ifs;
  std::size_t idx_r;
  
  constexpr read_variant_helper(T& field, F& field_list, std::ifstream& ifs, std::size_t idx_r) 
    : field(field), field_list(field_list), ifs(ifs), idx_r(idx_r) {}
  
  constexpr auto operator()() -> read_result {
    return (
      read_variant_impl<idx_head, field_head, F, typename field_head::field_type>(field, ifs, idx_r) |
      ... | 
      read_variant_impl<idx, fields, F, typename fields::field_type>(field, ifs, idx_r)
    );
  }
};


template <union_field_like T, field_list_like F>
struct read<T, F> {
  T& field;
  std::ifstream& ifs;

  constexpr read(T& field, std::ifstream& ifs): 
    field(field), ifs(ifs) {}

  constexpr auto operator()(F& field_list) -> read_result {
    using type_deduction_guide = typename T::type_deduction_guide;
    using field_choices = typename T::field_choices;
    using field_type = typename T::field_type;
    constexpr auto max_type_index = T::variant_size;

    constexpr auto type_index_deducer = type_deduction_guide();
    constexpr auto type_index_result = type_deduction_guide(field_list); 
    if(!type_index_result)
      return std::unexpected(type_index_result.error());
    constexpr auto idx_r = *type_index_result;
    constexpr auto field_read_res = 
      read_variant_helper<field_type, F, field_choices, std::make_index_sequence<max_type_index>>(field, field_list, ifs, idx_r);
    if(!field_read_res)
      return std::unexpected(field_read_res.error());
    return {};
  }
};

#endif // _FIELD_READER_HPP_
