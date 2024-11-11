#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_

#include <cstring>
#include <iostream>
#include <fstream>
#include <expected>
#include <utility>
#include "struct_field_list_traits.hpp"
#include "typelist.hpp"
#include "typelist_manip.hpp"


template <typename T>
auto read(const unsigned char* buffer, std::size_t size_to_read) 
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

template <std::size_t idx, typename variant_type, typename... types>
struct variant_reader_impl;

template <std::size_t idx, typename variant_type>
struct variant_reader_impl<idx, variant_type> {
  using result_type = std::expected<variant_type, std::string>;

  auto operator()(std::size_t, std::ifstream&, std::size_t) -> result_type {
    std::unreachable();
  }

  auto operator()(std::size_t, const unsigned char*, std::size_t) -> result_type {
    std::unreachable();
  }
};

template <std::size_t idx, typename variant_type, typename type_head, typename... type_tail>
struct variant_reader_impl<idx, variant_type, type_head, type_tail...> {
  using result_type = std::expected<variant_type, std::string>;

  auto operator()(std::size_t idx_r, std::ifstream& ifs, std::size_t size_to_read) -> result_type {
    if(idx_r == idx) {
      if constexpr(is_struct_field_list_v<type_head>) {
        auto res = struct_cast<type_head>(ifs);
        return (res) ? *res : std::unexpected("error");
      } else {
        type_head obj;
        if(!ifs.read(byte_addressof(obj), size_to_read))
          return std::unexpected("buffer exhaustion");
        return obj;
      }
    } else {
      return variant_reader_impl<idx + 1, variant_type, type_tail...>{}(idx_r, ifs, size_to_read);
    }
  }

  auto operator()(std::size_t idx_r, const unsigned char* buf, std::size_t size_to_read) -> result_type {
    if(idx_r == idx) {
      if constexpr(is_struct_field_list_v<type_head>) {
        auto res = struct_cast<type_head>(buf);
        return (res) ? *res : std::unexpected("error");
      } else {
        type_head obj;
        std::memcpy(to_void_ptr(obj), buf, size_to_read);
        return obj;
      }
    } else {
      return variant_reader_impl<idx + 1, variant_type, type_tail...>{}(idx_r, buf, size_to_read);
    }
  }
};

template <typename T>
struct variant_reader;

template <typename... types>
struct variant_reader<std::variant<types...>> {
  using variant_type = std::variant<types...>;
  using read_result = std::expected<variant_type, std::string>;

  auto operator()(std::size_t idx_r, std::istream& ifs, std::size_t size_to_read) -> read_result {
    return variant_reader_impl<0, variant_type, types...>{}(idx_r, ifs, size_to_read); 
  }

  auto operator()(std::size_t idx_r, const unsigned char* buf, std::size_t size_to_read) -> read_result {
    return variant_reader_impl<0, variant_type, types...>{}(idx_r, buf, size_to_read); 
  }
};

#endif // _FIELD_READER_HPP_
