#pragma once

#include <cstring>
#include <fstream>
#include <expected>
#include "sc_type_traits.hpp"


template <typename T>
auto read(const char* buffer, std::size_t size_to_read) 
    -> std::expected<T, std::string> {
  T obj;
  std::memcpy(to_void_ptr(obj), buffer, size_to_read);
  return obj;
}

// todo how to deduce return type
template <variant_like T, std::size_t idx, typename type_head, typename... type_tail>
auto read(const char* buffer, std::size_t type_idx_r) 
    -> std::expected<T, std::string> {
  if(type_idx_r == idx) {
    T variant_obj;
    typename type_head::type obj;
    std::memcpy(to_void_ptr(obj), buffer, type_head::size);
    variant_obj = obj;
    return variant_obj;
  } else {
    return read<T, idx - 1, type_tail...>(buffer, type_idx_r);
  }
}

// todo constraint type_tag like 
template <variant_like T, typename... types>
auto read(const char* buffer, std::size_t type_idx_r) 
    -> std::expected<T, std::string> {
  return read_impl<T, 0, types...>(buffer, type_idx_r);
}

template <typename T>
auto read(std::ifstream& ifs, std::size_t size_to_read) 
     -> std::expected<T, std::string> {
  T obj;
  if(!ifs.read(byte_addressof(obj), size_to_read))
    return std::unexpected("buffer exhaustion");
  return obj;
}

// todo swapt T and idx for better readability
template <variant_like T, std::size_t idx, typename type_head, typename... type_tail>
auto read(std::ifstream& ifs, std::size_t type_idx_r) 
    -> std::expected<T, std::string> {
  if(type_idx_r == idx) {
    T variant_obj;
    typename type_head::type obj;
    if(!ifs.read(byte_addressof(obj), type_head::size))
      return std::unexpected("buffer exhaustion");
    variant_obj = obj;
    return variant_obj;
  } else {
    return read<T, idx - 1, type_tail...>(ifs, type_idx_r);
  }
}

// todo constraint type_tag like 
template <variant_like T, typename... types>
auto read(std::ifstream& ifs, std::size_t type_idx_r) 
    -> std::expected<T, std::string> {
  return read_impl<T, 0, types...>(ifs, type_idx_r);
}
