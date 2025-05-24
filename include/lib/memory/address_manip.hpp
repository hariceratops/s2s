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

// todo generate this as configurable parameter
constexpr std::size_t constexpr_buffer_size = 2048;

template <identified_as_constexpr_stream stream, typename T, std::size_t size = sizeof(T)>
constexpr auto as_byte_buffer(T& obj) -> std::array<char, size> {
  return std::bit_cast<std::array<char, size>>(obj);
}
}

#endif // _ADDRESS_MANIP_HPP_
