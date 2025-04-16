#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_

#include <cstring>
#include <fstream>
#include <expected>
#include <utility>
#include "field_traits.hpp"
#include "field_meta.hpp"
#include "size_deduce.hpp"
#include "error.hpp"
#include "pipeline.hpp"
#include "field.hpp"
#include "stream.hpp"


// todo inheritance for ctor boilerplate removal: read<t,f>?
template <typename F, typename L>
struct read_field;

template <fixed_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;

  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list) {}
  
  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> read_result {
    using field_size = typename T::field_size;
    constexpr auto size_to_read = deduce_field_size<field_size>{}();
    return s.template read<endianness>(field.value, size_to_read);
  }
};


// todo what if vector elements are not aligned by 2
template <variable_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> read_result {
    using field_size = typename T::field_size;
    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    return s.template read<endianness>(field.value, len_to_read);
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

template <typename T, typename F, typename E>
struct read_buffer_of_records {
  T& field;
  F& field_list;
    std::size_t len_to_read;

  constexpr read_buffer_of_records(T& field, F& field_list, std::size_t len_to_read)
    : field(field), field_list(field_list), len_to_read(len_to_read) {}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> read_result {
    for(std::size_t count = 0; count < len_to_read; ++count) {
      // todo move E outside loop for optimization?
      E elem;
      auto reader = read_field<E, F>(elem, field_list);
      auto res = reader.template read<endianness, stream>(s);
      if(!res) 
        return std::unexpected(res.error());
      // todo is move guaranteed
      // todo optimise pass field.value[count] to reader instead of elem
      field.value[count] = std::move(elem.value);
      // field.value[count] = elem.value;
    }
    return {};
  }
};

template <array_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> read_result {
    using array_type = typename T::field_type;
    using array_element_field = create_field_from_array_of_records_v<T>;
    using read_impl_t = read_buffer_of_records<T, F, array_element_field>;

    constexpr auto array_len = extract_size_from_array_v<array_type>;
    auto reader = read_impl_t(field, field_list, array_len);
    auto res = reader.template read<endianness>(s);
    return res;
  }
};


template <vector_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> read_result {
    using vector_element_field = create_field_from_vector_of_records_v<T>;
    using field_size = typename T::field_size;
    using read_impl_t = read_buffer_of_records<T, F, vector_element_field>;

    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    field.value.resize(len_to_read);
    auto reader = read_impl_t(field, field_list, len_to_read);
    auto res = reader.template read<endianness>(s);
    return res;
  }
};


// Forward declaration
template <s2s_input_stream_like stream, field_list_like T, auto endianness>
constexpr auto struct_cast(stream&) -> std::expected<T, cast_error>;

template <struct_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> read_result {
    using field_list_t = extract_type_from_field_v<T>;
    auto res = struct_cast<stream, field_list_t, endianness>(s);
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
  
  constexpr read_field(T& field, F& field_list): 
    field(field), field_list(field_list){}
  
  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> read_result {
    if(!typename T::field_presence_checker{}(field_list)) {
      field.value = std::nullopt;
      return {};
    }
    using field_base_type_t = typename T::field_base_type;
    field_base_type_t base_field{};
    read_field<field_base_type_t, F> reader(base_field, field_list);
    auto res = reader.template read<endianness>(s);
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
  std::size_t idx_r;

  constexpr explicit read_variant_impl(
    V& variant, 
    F& field_list,
    std::size_t idx_r) :
      variant(variant), field_list(field_list), idx_r(idx_r) {}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> read_result {
    if (idx_r != idx) 
      return {};

    T field;
    auto reader = read_field<T, F>(field, field_list);
    auto res = reader.template read<endianness, stream>(s);
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
  std::size_t idx_r;
  
  constexpr read_variant_helper(T& field, F& field_list, std::size_t idx_r) 
    : field(field), field_list(field_list), idx_r(idx_r) {}
  
  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> read_result {
    read_result pipeline_seed{};
    return (
      pipeline_seed |
      ... | 
      [&]() {
        auto reader_impl = read_variant_impl<idx, fields, F, typename T::field_type>(field.value, field_list, idx_r);
        return reader_impl.template read<endianness>(s);
      }
    );
  }
};


// todo restore constexpr
template <union_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list): 
    field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> read_result {
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
    auto field_reader = read_helper_t(field, field_list, idx_r);
    auto field_read_res = field_reader.template read<endianness, stream>(s);
    if(!field_read_res)
      return std::unexpected(field_read_res.error());
    return {};
  }
};

#endif // _FIELD_READER_HPP_
