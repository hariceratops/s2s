#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_

#include <cstring>
#include <print>
#include <fstream>
#include <expected>
#include <utility>
#include "field_traits.hpp"
#include "field_meta.hpp"
#include "size_deduce.hpp"
#include "error.hpp"
#include "pipeline.hpp"

#include "field.hpp"

template <typename T>
// todo specialise for non scalar type to facilitate endianness specific vector read
constexpr auto raw_read(T& value, std::ifstream& ifs, std::size_t size_to_read) 
  -> read_result 
{
  if(!ifs.read(byte_addressof(value), size_to_read))
    return std::unexpected(cast_error::buffer_exhaustion);
  return {};
}


template <typename T>
constexpr auto read_scalar(T& value, std::size_t size_to_read, std::ifstream& ifs)
  -> read_result
{
  return raw_read(value, ifs, size_to_read);
}


template <typename T>
constexpr auto read_vector(T& value, std::size_t len_to_read, std::ifstream& ifs) 
  -> read_result 
{
  constexpr auto size_of_one_elem = sizeof(T{}[0]);
  value.resize(len_to_read);
  return raw_read(value, ifs, len_to_read * size_of_one_elem);
}


// todo inheritance for ctor boilerplate removal: read<t,f>?
template <typename F, typename L>
struct read_field;

template <fixed_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using field_size = typename T::field_size;
    constexpr auto size_to_read = deduce_field_size<field_size>{}();
    return read_scalar(field.value, size_to_read, ifs);
  }
};


template <variable_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using field_size = typename T::field_size;
    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    return read_vector(field.value, len_to_read, ifs);
  }
};


struct not_array_of_records_field {};

template <typename T>
struct create_field_from_array_of_records;

template <array_of_record_field_like T>
struct create_field_from_array_of_records<T> {
  using array_type = typename T::field_type;
  using array_elem_type = extract_type_from_array_v<array_type>;
  static constexpr auto field_id = T::field_id;
  using size = field_size<size_dont_care>;
  static constexpr auto constraint = no_constraint<array_elem_type>{};

  using res = field<field_id, array_elem_type, size, constraint>;
};

template <typename T>
using create_field_from_array_of_records_v = create_field_from_array_of_records<T>::res;

template <array_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using array_type = typename T::field_type;
    using array_element_field = create_field_from_array_of_records_v<T>;
    constexpr auto array_len = extract_size_from_array_v<array_type>;

    for(std::size_t count = 0; count < array_len; ++count) {
      array_element_field t;
      auto reader = read_field<array_element_field, F>(t, field_list, ifs);
      auto res = reader();
      if(!res) 
        return std::unexpected(res.error());
      field.value[count] = t.value;
    }
    return {};
  }
};


struct not_vector_of_records_field {};

template <typename T>
struct create_field_from_vector_of_records;

template <vector_of_record_field_like T>
struct create_field_from_vector_of_records<T> {
  using vector_type = typename T::field_type;
  using vector_elem_type = extract_type_from_vec_t<vector_type>;
  static constexpr auto field_id = T::field_id;
  using size = field_size<size_dont_care>;
  static constexpr auto constraint = no_constraint<vector_elem_type>{};

  using res = field<field_id, vector_elem_type, size, constraint>;
};

template <typename T>
using create_field_from_vector_of_records_v = create_field_from_vector_of_records<T>::res;

template <vector_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using vector_element_field = create_field_from_vector_of_records_v<T>;
    using field_size = typename T::field_size;

    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    field.value.resize(len_to_read);

    for(std::size_t count = 0; count < len_to_read; ++count) {
      vector_element_field t;
      auto reader = read_field<vector_element_field, F>(t, field_list, ifs);
      auto res = reader();
      if(!res) 
        return std::unexpected(res.error());
      field.value[count] = t.value;
    }
    return {};
  }
};


// Forward declaration
template <field_list_like T>
constexpr auto struct_cast(std::ifstream&) -> std::expected<T, cast_error>;

template <struct_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using field_list_t = extract_type_from_field_v<T>;
    auto res = struct_cast<field_list_t>(ifs);
    if(!res)
      return std::unexpected(res.error());
    // todo move?
    field.value = *res;
    return {};
  }
};


// todo restore constexpr
template <optional_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs): 
    field(field), field_list(field_list), ifs(ifs) {}
  
  constexpr auto operator()() -> read_result {
    if(!typename T::field_presence_checker{}(field_list)) {
      field.value = std::nullopt;
      return {};
    }
    using field_base_type_t = typename T::field_base_type;
    field_base_type_t base_field{};
    read_field<field_base_type_t, F> reader(base_field, field_list, ifs);
    auto res = reader();
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
  V& variant;
  F& field_list;
  std::ifstream& ifs;
  std::size_t idx_r;

  constexpr explicit read_variant_impl(
    V& variant, 
    F& field_list,
    std::ifstream& ifs, 
    std::size_t idx_r) :
      variant(variant), field_list(field_list), ifs(ifs), idx_r(idx_r) {}

  constexpr auto operator()() -> read_result {
    if (idx_r != idx) 
      return {};

    T field;
    auto reader = read_field<T, F>(field, field_list, ifs);
    auto res = reader();
    if(!res)
      return std::unexpected(res.error());
    variant = std::move(field.value);
    return {};
  }
};


template <typename T, typename F, typename field_choices, typename idx_seq>
struct read_variant_helper;

template <typename T, typename F, typename... fields, std::size_t... idx>
struct read_variant_helper<T, F, field_choice_list<fields...>, std::index_sequence<idx...>> {
  T& field;
  F& field_list;
  std::ifstream& ifs;
  std::size_t idx_r;
  
  constexpr read_variant_helper(T& field, F& field_list, std::ifstream& ifs, std::size_t idx_r) 
    : field(field), field_list(field_list), ifs(ifs), idx_r(idx_r) {}
  
  constexpr auto operator()() -> read_result {
    read_result pipeline_seed{};
    return (
      pipeline_seed |
      ... | 
      read_variant_impl<idx, fields, F, typename T::field_type>(field.value, field_list, ifs, idx_r)
    );
  }
};


// todo restore constexpr
template <union_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs): 
    field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() -> read_result {
    using type_deduction_guide = typename T::type_deduction_guide;
    using field_choices = typename T::field_choices;
    constexpr auto max_type_index = T::variant_size;

    auto type_index_deducer = type_deduction_guide();
    auto type_index_result = type_index_deducer(field_list); 
    if(!type_index_result)
      return std::unexpected(type_index_result.error());

    auto idx_r = *type_index_result;
    using read_helper_t = 
      read_variant_helper<
        T, 
        F, 
        field_choices, 
        std::make_index_sequence<max_type_index>
      >;
    auto field_reader = read_helper_t(field, field_list, ifs, idx_r);
    auto field_read_res = field_reader();
    if(!field_read_res)
      return std::unexpected(field_read_res.error());
    return {};
  }
};

#endif // _FIELD_READER_HPP_
