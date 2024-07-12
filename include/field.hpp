#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#include <iostream>
#include "fixed_string.hpp"
#include "field_base.hpp"
#include "address_manip.hpp"
#include "sc_type_traits.hpp"
#include "field_constraints.hpp"
#include "field_size.hpp"
#include <fstream>
#include <cstring>
#include <array>
#include <vector>
#include <string>

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

template <fixed_string id,
          typename T,
          auto constraint = no_constraint<T>{}> 
struct basic_field: public field_base<id, T> {
  void read(const char* buffer, std::size_t size_to_read) {
    std::memcpy(to_void_ptr(this->value), buffer, size_to_read);
    assert(constraint(this->value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    ifs.read(byte_addressof(this->value), size_to_read);
    assert(constraint(this->value));
  }
};

template <fixed_string id,
          field_containable T,
          typename comptime_size,
          auto constraint = no_constraint<T>{}> 
struct field: public basic_field<id, T> {
  static constexpr auto field_size = comptime_size::size;
};

// todo remove field_containable since value of T type shall
// be either allocated or managed, constain accordingly
// todo is contraint required? maybe.
template <fixed_string id,
          typename T,
          typename runtime_size,
          auto constraint = no_constraint<T>{}> 
struct runtime_field: public basic_field<id, T> {
  static constexpr auto field_accessor = runtime_size::accessor;
  void read(const char* buffer, std::size_t size_to_read) {
    this->value.resize(size_to_read);
    std::memcpy(to_void_ptr(this->value), buffer, size_to_read);
    assert(constraint(this->value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    this->value.resize(size_to_read);
    ifs.read(byte_addressof(this->value), size_to_read);
    assert(constraint(this->value));
  }
};


template <fixed_string id, field_list_like T>
struct struct_field : field_base<id, T> {
  static constexpr auto field_size = sizeof(T);
};

// Aliases
// todo enforce constraints wherever applicable
template <fixed_string id, typename T, std::size_t N>
using fixed_array_field = field<id, std::array<T, N>, field_size<N * sizeof(T)>>;

template <fixed_string id, std::size_t N>
using fixed_string_field = field<id, fixed_string<N + 1>, field_size<N + 1>>;

template <fixed_string id, typename T, std::size_t N>
using c_arr_field = field<id, T[N], field_size<N * sizeof(T)>>;

template <fixed_string id, std::size_t N>
using c_str_field = field<id, char[N + 1], field_size<N * sizeof(char) + 1>>;

template <fixed_string id, floating_point T>
using float_point_field = field<id, T, field_size<sizeof(T)>>;

template <fixed_string id, std::size_t N, std::array<unsigned char, N> expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, field_size<N>, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, field_size<expected.size()>, eq{expected}>;

template <fixed_string id, integral T, std::size_t size, T expected>
using magic_number = field<id, T, field_size<size>, eq{expected}>;

// digressions = What if user wants a custom allocator? use the plain version of the type instead of alias?
// todo get vector length in bytes instead of size to read additional overload
template <fixed_string id, typename T, typename runtime_size>
using vec_field = runtime_field<id, std::vector<T>, runtime_size>;

// todo check if this will work for all char types like wstring
template <fixed_string id, typename runtime_size>
using str_field = runtime_field<id, std::string, runtime_size>;

#endif
