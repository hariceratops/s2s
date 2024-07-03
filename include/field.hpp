#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#include "fixed_string.hpp"
#include "field_base.hpp"
#include "address_manip.hpp"
#include "sc_type_traits.hpp"
#include "field_constraints.hpp"
#include <fstream>
#include <cstring>


template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

template <fixed_string id,
          field_containable T,
          std::size_t size,
          auto constraint = no_constraint<T>{}> 
struct field: public field_base<id, T, size> {
  void read(const char* buffer, std::size_t size_to_read) {
    std::memcpy(to_void_ptr(this->value), buffer, size_to_read);
    assert(constraint(this->value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    ifs.read(byte_addressof(this->value), size_to_read);
    assert(constraint(this->value));
  }
};


template <fixed_string id, field_list_like T>
struct struct_field : field_base<id, T, sizeof(T)> {};

// Aliases
template <fixed_string id, typename T, std::size_t N>
using fixed_array_field = field<id, std::array<T, N>, N * sizeof(T)>;

template <fixed_string id, std::size_t N>
using fixed_string_field = field<id, fixed_string<N + 1>, N + 1>;

template <fixed_string id, typename T, std::size_t N>
using c_arr_field = field<id, T[N], N * sizeof(T)>;

template <fixed_string id, std::size_t N>
using c_str_field = field<id, char[N + 1], N * sizeof(char) + 1>;

template <fixed_string id, floating_point T>
using float_point_field = field<id, T, sizeof(T)>;

template <fixed_string id, std::size_t N, std::array<unsigned char, N> expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, N, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, expected.size(), eq{expected}>;

template <fixed_string id, integral T, std::size_t size, T expected>
using magic_number = field<id, T, size, eq{expected}>;

#endif
