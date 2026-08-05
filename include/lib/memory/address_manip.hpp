#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_


#include "../../stream/stream_traits.hpp"
#include "../containers/fixed_string.hpp"
#include <vector>
#include <array>


namespace s2s {
template <input_stream_like stream, typename T>
char* byte_addressof(T& obj) {
  return reinterpret_cast<char*>(&obj);
}

template <input_stream_like stream, typename T, std::size_t N>
char* byte_addressof(std::array<T, N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <input_stream_like stream, std::size_t N>
char* byte_addressof(fixed_string<N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <input_stream_like stream, typename T>
char* byte_addressof(std::vector<T>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <input_stream_like stream>
inline char* byte_addressof(std::string& obj) {
  return reinterpret_cast<char*>(&obj[0]);
}

template <output_stream_like stream, typename T>
const char* const_byte_addressof(const T& obj) {
  return reinterpret_cast<const char*>(&obj);
}

template <output_stream_like stream, typename T, std::size_t N>
const char* const_byte_addressof(const std::array<T, N>& obj) {
  return reinterpret_cast<const char*>(obj.data());
}

template <output_stream_like stream, std::size_t N>
const char* const_byte_addressof(const fixed_string<N>& obj) {
  return reinterpret_cast<const char*>(obj.data());
}

template <output_stream_like stream, typename T>
const char* const_byte_addressof(const std::vector<T>& obj) {
  return reinterpret_cast<const char*>(obj.data());
}

template <output_stream_like stream>
inline const char* const_byte_addressof(const std::string& obj) {
  return reinterpret_cast<const char*>(obj.data());
}

// todo generate this as configurable parameter
constexpr std::size_t constexpr_buffer_size = 2048;

template <identified_as_constexpr_stream stream, typename T, std::size_t size = sizeof(T)>
constexpr auto as_byte_buffer(const T& obj) -> std::array<char, size> {
  return std::bit_cast<std::array<char, size>>(obj);
}
}

#endif // _ADDRESS_MANIP_HPP_
