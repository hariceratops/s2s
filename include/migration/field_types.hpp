#ifndef _FIELD_TYPE_HPP_
#define _FIELD_TYPE_HPP_


#include "sc_type_traits.hpp"
#include "field.hpp"
#include "field_size.hpp"
#include "compute_res.hpp"



// Aliases
// todo enforce constraints wherever applicable
//
template <fixed_string id, 
          integral T, 
          comptime_size_like size, 
          auto expected = no_constraint<T>{}>
using integral_field = field<id, T, size, expected>;

template <fixed_string id, 
          typename T, 
          std::size_t N, 
          auto expected = no_constraint<T>{}>
using fixed_array_field = field<id, std::array<T, N>, field_size<N * sizeof(T)>, expected>;

template <fixed_string id, 
          std::size_t N>
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

#endif /* _FIELD_TYPE_HPP_ */
