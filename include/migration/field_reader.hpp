#pragma once

#include <cstring>
#include <fstream>
#include <expected>


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
