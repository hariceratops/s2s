#pragma once

#include <cstring>
#include <fstream>
#include <expected>
#include "field.hpp"
#include "sc_type_traits.hpp"
#include "../typelist.hpp"
#include "typelist_manip.hpp"


template <std::size_t idx, typename T, typename U, typename V>
struct variant_reader_impl;

template <std::size_t idx, typename T>
struct variant_reader_impl<idx, T, typelist::typelist<>, std::integer_sequence<std::size_t>> {
  auto operator()([[maybe_unused]] std::size_t idx_r, [[maybe_unused]]std::ifstream& ifs) 
    -> std::expected<std::variant<T>, std::string> {
    std::unexpected("Out of bounds");
  }

  auto operator()([[maybe_unused]] std::size_t idx_r, [[maybe_unused]]const char* buf) 
    -> std::expected<std::variant<T>, std::string> {
    std::unexpected("Out of bounds");
  }
};

template <std::size_t idx, typename T, typename... types, std::size_t... sizes>
struct variant_reader_impl<idx, 
                           T, 
                           typelist::typelist<types...>, 
                           std::integer_sequence<std::size_t, sizes...>> 
{
  using type_to_read = tl_front_t<typelist::typelist<types...>>;
  static constexpr std::size_t size_to_read = iseq_front_v<std::integer_sequence<std::size_t, sizes...>>;

  auto operator()(std::size_t idx_r, std::ifstream& ifs) -> 
    std::expected<std::variant<T>, std::string> {
    if(idx_r == idx) {
      if constexpr(is_struct_field_list_v<type_to_read>) {
        auto res = struct_cast<front_t<types>>(buf);
        return (res) ? *res : std::unexpected("error");
      } else {
        type_to_read obj;
        if(!ifs.read(byte_addressof(obj), size_to_read))
          return std::unexpected("buffer exhaustion");
        return obj;
      }
    } else {
      return 
        variant_reader_impl<
         idx + 1, 
         T, 
         tl_pop_t<typelist::typelist<types...>, 
         iseq_pop_t<std::integer_sequence<std::size_t, sizes...>>
       >(idx_r, buf)
    }
  }

  auto operator()(std::size_t idx_r, const char* buf) -> 
    std::expected<std::variant<T>, std::string> {
    if(idx_r == idx) {
      if constexpr(is_struct_field_list_v<type_to_read>) {
        auto res = struct_cast<front_t<types>>(buf);
        return (res) ? *res : std::unexpected("error");
      } else {
        type_to_read obj;
        std::memcpy(to_void_ptr(obj), buffer, size_to_read);
      }
    } else {
      return 
        variant_reader_impl<
         idx + 1, 
         T, 
         tl_pop_t<typelist::typelist<types...>, 
         iseq_pop_t<std::integer_sequence<std::size_t, sizes...>>
       >(idx_r, buf)
    }
  }
};

template <typename T, typename U>
struct variant_reader;

template <typename T, std::size_t... sizes>
struct variant_reader<T, std::integer_sequence<std::size_t, sizes...>> {
  using types = variant_to_typelist_t<T>;
  static constexpr auto size_list = std::integer_sequence<std::size_t, sizes...>{};
  using read_result = std::expected<T, std::string>;

  auto operator()(std::size_t idx_r, std::istream& ifs) -> read_result {
    variant_reader_impl<0, T, types, size_list>(idx_r, ifs); 
  }

  auto operator()(std::size_t idx_r, const char* buf) -> read_result {
    variant_reader_impl<0, T, types, size_list>(idx_r, buf); 
  }
};

template <typename T>
auto read(const char* buffer, std::size_t size_to_read) 
    -> std::expected<T, std::string> {
  T obj;
  std::memcpy(to_void_ptr(obj), buffer, size_to_read);
  return obj;
}

template <typename T>
auto read(std::ifstream& ifs, std::size_t size_to_read) 
     -> std::expected<T, std::string> {
  T obj;
  if(!ifs.read(byte_addressof(obj), size_to_read))
    return std::unexpected("buffer exhaustion");
  return obj;
}

