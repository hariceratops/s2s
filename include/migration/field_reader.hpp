#pragma once

#include <cstring>
#include <fstream>
#include <expected>


template <typename T>
auto read(T& dest, const char* buffer, std::size_t size_to_read) -> std::expected<void, std::string> {
  std::memcpy(to_void_ptr(dest), buffer, size_to_read);
  return {};
}

template <typename T>
auto read(T& dest, std::ifstream& ifs, std::size_t size_to_read) 
     -> std::expected<void, std::string> {
  if(!ifs.read(byte_addressof(dest), size_to_read))
    return std::unexpected("buffer exhaustion");
  return {};
}
