#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#include <array>
#include <string>
#include <vector>
#include "fixed_string.hpp"


template <typename T>
void* to_void_ptr(T& obj) {
  return reinterpret_cast<void*>(&obj);
}

template <typename T, std::size_t N>
void* to_void_ptr(std::array<T, N>& obj) {
  return reinterpret_cast<void*>(obj.data());
}

template <std::size_t N>
void* to_void_ptr(fixed_string<N>& obj) {
  return reinterpret_cast<void*>(obj.data());
}

template <typename T>
void* to_void_ptr(std::vector<T>& obj) {
  return reinterpret_cast<void*>(obj.data());
}

template <typename T>
void* to_void_ptr(std::string& obj) {
  return reinterpret_cast<void*>(obj.data());
}

template <typename T>
char* byte_addressof(T& obj) {
  return reinterpret_cast<char*>(&obj);
}

template <typename T, std::size_t N>
char* byte_addressof(std::array<T, N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <std::size_t N>
char* byte_addressof(fixed_string<N>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

template <typename T>
char* byte_addressof(std::vector<T>& obj) {
  return reinterpret_cast<char*>(obj.data());
}

inline char* byte_addressof(std::string& obj) {
  return reinterpret_cast<char*>(&obj[0]);
}

#endif // _ADDRESS_MANIP_HPP_
