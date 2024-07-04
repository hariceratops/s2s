#ifndef STRUCT_CAST_HPP
#define STRUCT_CAST_HPP

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#include <array>

// Function to void pointer cast
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

// Function to get byte address
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

#endif // _ADDRESS_MANIP_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#include <array>

// Function to void pointer cast
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

// Function to get byte address
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

#endif // _ADDRESS_MANIP_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

namespace tl = typelist;

// Concept for strict callable
template <typename T, typename Arg>
concept strict_callable = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<T, typename std::remove_cvref_t<Arg>>;

// Concepts
template <typename T>
concept equality_comparable = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
    { a != b } -> std::same_as<bool>;
} && !std::is_floating_point_v<T>;

template <typename T>
concept comparable = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
    { a <= b } -> std::same_as<bool>;
    { a >= b } -> std::same_as<bool>;
};

template <typename T>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || is_fixed_string_v<T>);

// Structs for predefined constraints
template <equality_comparable T>
struct eq {
  T v;
  constexpr eq(T value) : v(value) {}
  constexpr eq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v == v; }
};

template <equality_comparable T>
struct neq {
  T v;
  constexpr neq(T value) : v(value) {}
  constexpr neq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v != v; }
};

template <comparable T>
struct lt {
  T v;
  constexpr lt(T value) : v(value) {}
  constexpr lt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v < v; }
};

template <comparable T>
struct gt {
  T v;
  constexpr gt(T value) : v(value) {}
  constexpr gt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v > v; }
};

template <typename T>
struct lte {
  T v;
  constexpr lte(T value) : v(value) {}
  constexpr lte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v <= v; }
};

template <inequality_comparable T>
struct gte {
  T v;
  constexpr gte(T value) : v(value) {}
  constexpr gte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v >= v; }
};

template <typename T>
struct no_constraint {
  constexpr bool operator()([[maybe_unused]] const T& actual_v) const { 
    return true; 
  }
};

template <typename T, typename... Ts>
  requires (tl::all_are_same_v<tl::typelist<T, Ts...>>)
struct any_of {
  std::array<T, 1 + sizeof...(Ts)> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    return std::find(possible_values.begin(), 
                     possible_values.end(), 
                     actual_v) != possible_values.end();
  }
};

// Range struct
template <typename T>
struct range {
  T a;
  T b;

  constexpr range(T value1, T value2) : a(value1), b(value2) {
    static_assert(value1 < value2, "Range start must be less than range end");
  }
};

// CTAD for range
template <typename T>
range(T, T) -> range<T>;

// Struct to check if a value is in any of the open intervals
template <typename t, typename... ts>
  requires (tl::all_are_same_v<tl::typelist<ts...>>)
struct is_in_open_range {
  std::array<range<t>, 1 + sizeof...(ts)> open_ranges;

  constexpr is_in_open_range(range<t> range, ::range<ts>... ranges) : open_ranges{range, ranges...} {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const ::range<t>& r1, const ::range<t>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const t& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<t>& r, const t& v) {
      return r.b < v;
    });
    if (it != open_ranges.begin() && (it == open_ranges.end() || it->a > value)) {
      --it;
    }
    return it != open_ranges.end() && it->a < value && value < it->b;
  }
};

// Struct to check if a value is in any of the closed intervals
template <typename T, std::size_t N>
struct is_in_closed_range {
  std::array<range<T>, N> closed_ranges;

  constexpr is_in_closed_range(std::array<range<T>, N> ranges) : closed_ranges(ranges) {
    std::sort(closed_ranges.begin(), closed_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(closed_ranges.begin(), closed_ranges.end(), value, [](const range<T>& r, const T& v) {
      return r.b < v;
    });
    if (it != closed_ranges.begin() && (it == closed_ranges.end() || it->a > value)) {
      --it;
    }
    return it != closed_ranges.end() && it->a <= value && value <= it->b;
  }
};

// CTAD (Class Template Argument Deduction) guides
template <typename T>
eq(T) -> eq<T>;

template <typename T>
neq(T) -> neq<T>;

template <typename T>
lt(T) -> lt<T>;

template <typename T>
gt(T) -> gt<T>;

template <typename T>
lte(T) -> lte<T>;

template <typename T>
gte(T) -> gte<T>;

template <typename t, typename... ts> 
any_of(t, ts...) -> any_of<t, ts...>;

template <typename t, typename... ts> 
is_in_open_range(range<t>, range<ts>...) -> is_in_open_range<t, ts...>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

// #include "field_size.hpp"
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
template <fixed_string id,
          typename T,
          typename runtime_size,
          auto constraint = no_constraint<T>{}> 
struct runtime_field: public basic_field<id, T> {
  static constexpr auto field_accessor = runtime_size::accessor;
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

// What if user wants a custom allocator
template <fixed_string id, typename T, typename runtime_size>
using vec_field = runtime_field<id, std::vector<T>, runtime_size>;

template <fixed_string id, typename runtime_size>
using str_field = runtime_field<id, std::string, runtime_size>;

#endif

#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#include <array>

// Function to void pointer cast
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

// Function to get byte address
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

#endif // _ADDRESS_MANIP_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

namespace tl = typelist;

// Concept for strict callable
template <typename T, typename Arg>
concept strict_callable = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<T, typename std::remove_cvref_t<Arg>>;

// Concepts
template <typename T>
concept equality_comparable = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
    { a != b } -> std::same_as<bool>;
} && !std::is_floating_point_v<T>;

template <typename T>
concept comparable = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
    { a <= b } -> std::same_as<bool>;
    { a >= b } -> std::same_as<bool>;
};

template <typename T>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || is_fixed_string_v<T>);

// Structs for predefined constraints
template <equality_comparable T>
struct eq {
  T v;
  constexpr eq(T value) : v(value) {}
  constexpr eq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v == v; }
};

template <equality_comparable T>
struct neq {
  T v;
  constexpr neq(T value) : v(value) {}
  constexpr neq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v != v; }
};

template <comparable T>
struct lt {
  T v;
  constexpr lt(T value) : v(value) {}
  constexpr lt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v < v; }
};

template <comparable T>
struct gt {
  T v;
  constexpr gt(T value) : v(value) {}
  constexpr gt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v > v; }
};

template <typename T>
struct lte {
  T v;
  constexpr lte(T value) : v(value) {}
  constexpr lte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v <= v; }
};

template <inequality_comparable T>
struct gte {
  T v;
  constexpr gte(T value) : v(value) {}
  constexpr gte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v >= v; }
};

template <typename T>
struct no_constraint {
  constexpr bool operator()([[maybe_unused]] const T& actual_v) const { 
    return true; 
  }
};

template <typename T, typename... Ts>
  requires (tl::all_are_same_v<tl::typelist<T, Ts...>>)
struct any_of {
  std::array<T, 1 + sizeof...(Ts)> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    return std::find(possible_values.begin(), 
                     possible_values.end(), 
                     actual_v) != possible_values.end();
  }
};

// Range struct
template <typename T>
struct range {
  T a;
  T b;

  constexpr range(T value1, T value2) : a(value1), b(value2) {
    static_assert(value1 < value2, "Range start must be less than range end");
  }
};

// CTAD for range
template <typename T>
range(T, T) -> range<T>;

// Struct to check if a value is in any of the open intervals
template <typename t, typename... ts>
  requires (tl::all_are_same_v<tl::typelist<ts...>>)
struct is_in_open_range {
  std::array<range<t>, 1 + sizeof...(ts)> open_ranges;

  constexpr is_in_open_range(range<t> range, ::range<ts>... ranges) : open_ranges{range, ranges...} {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const ::range<t>& r1, const ::range<t>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const t& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<t>& r, const t& v) {
      return r.b < v;
    });
    if (it != open_ranges.begin() && (it == open_ranges.end() || it->a > value)) {
      --it;
    }
    return it != open_ranges.end() && it->a < value && value < it->b;
  }
};

// Struct to check if a value is in any of the closed intervals
template <typename T, std::size_t N>
struct is_in_closed_range {
  std::array<range<T>, N> closed_ranges;

  constexpr is_in_closed_range(std::array<range<T>, N> ranges) : closed_ranges(ranges) {
    std::sort(closed_ranges.begin(), closed_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(closed_ranges.begin(), closed_ranges.end(), value, [](const range<T>& r, const T& v) {
      return r.b < v;
    });
    if (it != closed_ranges.begin() && (it == closed_ranges.end() || it->a > value)) {
      --it;
    }
    return it != closed_ranges.end() && it->a <= value && value <= it->b;
  }
};

// CTAD (Class Template Argument Deduction) guides
template <typename T>
eq(T) -> eq<T>;

template <typename T>
neq(T) -> neq<T>;

template <typename T>
lt(T) -> lt<T>;

template <typename T>
gt(T) -> gt<T>;

template <typename T>
lte(T) -> lte<T>;

template <typename T>
gte(T) -> gte<T>;

template <typename t, typename... ts> 
any_of(t, ts...) -> any_of<t, ts...>;

template <typename t, typename... ts> 
is_in_open_range(range<t>, range<ts>...) -> is_in_open_range<t, ts...>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

// #include "field_size.hpp"
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
template <fixed_string id,
          typename T,
          typename runtime_size,
          auto constraint = no_constraint<T>{}> 
struct runtime_field: public basic_field<id, T> {
  static constexpr auto field_accessor = runtime_size::accessor;
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

// What if user wants a custom allocator
template <fixed_string id, typename T, typename runtime_size>
using vec_field = runtime_field<id, std::vector<T>, runtime_size>;

template <fixed_string id, typename runtime_size>
using str_field = runtime_field<id, std::string, runtime_size>;

#endif

#ifndef _FIELD_LOOKUP_HPP_
#define _FIELD_LOOKUP_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#include <array>

// Function to void pointer cast
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

// Function to get byte address
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

#endif // _ADDRESS_MANIP_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

namespace tl = typelist;

// Concept for strict callable
template <typename T, typename Arg>
concept strict_callable = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<T, typename std::remove_cvref_t<Arg>>;

// Concepts
template <typename T>
concept equality_comparable = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
    { a != b } -> std::same_as<bool>;
} && !std::is_floating_point_v<T>;

template <typename T>
concept comparable = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
    { a <= b } -> std::same_as<bool>;
    { a >= b } -> std::same_as<bool>;
};

template <typename T>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || is_fixed_string_v<T>);

// Structs for predefined constraints
template <equality_comparable T>
struct eq {
  T v;
  constexpr eq(T value) : v(value) {}
  constexpr eq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v == v; }
};

template <equality_comparable T>
struct neq {
  T v;
  constexpr neq(T value) : v(value) {}
  constexpr neq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v != v; }
};

template <comparable T>
struct lt {
  T v;
  constexpr lt(T value) : v(value) {}
  constexpr lt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v < v; }
};

template <comparable T>
struct gt {
  T v;
  constexpr gt(T value) : v(value) {}
  constexpr gt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v > v; }
};

template <typename T>
struct lte {
  T v;
  constexpr lte(T value) : v(value) {}
  constexpr lte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v <= v; }
};

template <inequality_comparable T>
struct gte {
  T v;
  constexpr gte(T value) : v(value) {}
  constexpr gte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v >= v; }
};

template <typename T>
struct no_constraint {
  constexpr bool operator()([[maybe_unused]] const T& actual_v) const { 
    return true; 
  }
};

template <typename T, typename... Ts>
  requires (tl::all_are_same_v<tl::typelist<T, Ts...>>)
struct any_of {
  std::array<T, 1 + sizeof...(Ts)> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    return std::find(possible_values.begin(), 
                     possible_values.end(), 
                     actual_v) != possible_values.end();
  }
};

// Range struct
template <typename T>
struct range {
  T a;
  T b;

  constexpr range(T value1, T value2) : a(value1), b(value2) {
    static_assert(value1 < value2, "Range start must be less than range end");
  }
};

// CTAD for range
template <typename T>
range(T, T) -> range<T>;

// Struct to check if a value is in any of the open intervals
template <typename t, typename... ts>
  requires (tl::all_are_same_v<tl::typelist<ts...>>)
struct is_in_open_range {
  std::array<range<t>, 1 + sizeof...(ts)> open_ranges;

  constexpr is_in_open_range(range<t> range, ::range<ts>... ranges) : open_ranges{range, ranges...} {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const ::range<t>& r1, const ::range<t>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const t& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<t>& r, const t& v) {
      return r.b < v;
    });
    if (it != open_ranges.begin() && (it == open_ranges.end() || it->a > value)) {
      --it;
    }
    return it != open_ranges.end() && it->a < value && value < it->b;
  }
};

// Struct to check if a value is in any of the closed intervals
template <typename T, std::size_t N>
struct is_in_closed_range {
  std::array<range<T>, N> closed_ranges;

  constexpr is_in_closed_range(std::array<range<T>, N> ranges) : closed_ranges(ranges) {
    std::sort(closed_ranges.begin(), closed_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(closed_ranges.begin(), closed_ranges.end(), value, [](const range<T>& r, const T& v) {
      return r.b < v;
    });
    if (it != closed_ranges.begin() && (it == closed_ranges.end() || it->a > value)) {
      --it;
    }
    return it != closed_ranges.end() && it->a <= value && value <= it->b;
  }
};

// CTAD (Class Template Argument Deduction) guides
template <typename T>
eq(T) -> eq<T>;

template <typename T>
neq(T) -> neq<T>;

template <typename T>
lt(T) -> lt<T>;

template <typename T>
gt(T) -> gt<T>;

template <typename T>
lte(T) -> lte<T>;

template <typename T>
gte(T) -> gte<T>;

template <typename t, typename... ts> 
any_of(t, ts...) -> any_of<t, ts...>;

template <typename t, typename... ts> 
is_in_open_range(range<t>, range<ts>...) -> is_in_open_range<t, ts...>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

// #include "field_size.hpp"
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
template <fixed_string id,
          typename T,
          typename runtime_size,
          auto constraint = no_constraint<T>{}> 
struct runtime_field: public basic_field<id, T> {
  static constexpr auto field_accessor = runtime_size::accessor;
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

// What if user wants a custom allocator
template <fixed_string id, typename T, typename runtime_size>
using vec_field = runtime_field<id, std::vector<T>, runtime_size>;

template <fixed_string id, typename runtime_size>
using str_field = runtime_field<id, std::string, runtime_size>;

#endif

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, typename T, typename size, auto field_constraint, typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint>, rest...>, id> {
  using type = field<id, T, size, field_constraint>;
};

template <fixed_string id, typename T, typename... rest>
struct field_lookup<field_list<struct_field<id, T>, rest...>, id> {
  using type = struct_field<id, T>;
};

template <fixed_string id, typename T, typename size, auto field_constraint, typename... rest>
struct field_lookup<field_list<runtime_field<id, T, size, field_constraint>, rest...>, id> {
  using type = struct_field<id, T>;
};

template <fixed_string id, typename head, typename... rest>
struct field_lookup<field_list<head, rest...>, id> {
  using type = typename field_lookup<field_list<rest...>, id>::type;
};

template <fixed_string id>
struct field_lookup<field_list<>, id> {
  using type = field_lookup_failed;
};

template <typename ls, fixed_string id>
using field_lookup_v = typename field_lookup<ls, id>::type;

#endif // _FIELD_LOOKUP_HPP_

#ifndef _FIELD_META_HPP_
#define _FIELD_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#include <array>

// Function to void pointer cast
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

// Function to get byte address
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

#endif // _ADDRESS_MANIP_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

namespace tl = typelist;

// Concept for strict callable
template <typename T, typename Arg>
concept strict_callable = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<T, typename std::remove_cvref_t<Arg>>;

// Concepts
template <typename T>
concept equality_comparable = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
    { a != b } -> std::same_as<bool>;
} && !std::is_floating_point_v<T>;

template <typename T>
concept comparable = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
    { a <= b } -> std::same_as<bool>;
    { a >= b } -> std::same_as<bool>;
};

template <typename T>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || is_fixed_string_v<T>);

// Structs for predefined constraints
template <equality_comparable T>
struct eq {
  T v;
  constexpr eq(T value) : v(value) {}
  constexpr eq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v == v; }
};

template <equality_comparable T>
struct neq {
  T v;
  constexpr neq(T value) : v(value) {}
  constexpr neq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v != v; }
};

template <comparable T>
struct lt {
  T v;
  constexpr lt(T value) : v(value) {}
  constexpr lt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v < v; }
};

template <comparable T>
struct gt {
  T v;
  constexpr gt(T value) : v(value) {}
  constexpr gt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v > v; }
};

template <typename T>
struct lte {
  T v;
  constexpr lte(T value) : v(value) {}
  constexpr lte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v <= v; }
};

template <inequality_comparable T>
struct gte {
  T v;
  constexpr gte(T value) : v(value) {}
  constexpr gte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v >= v; }
};

template <typename T>
struct no_constraint {
  constexpr bool operator()([[maybe_unused]] const T& actual_v) const { 
    return true; 
  }
};

template <typename T, typename... Ts>
  requires (tl::all_are_same_v<tl::typelist<T, Ts...>>)
struct any_of {
  std::array<T, 1 + sizeof...(Ts)> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    return std::find(possible_values.begin(), 
                     possible_values.end(), 
                     actual_v) != possible_values.end();
  }
};

// Range struct
template <typename T>
struct range {
  T a;
  T b;

  constexpr range(T value1, T value2) : a(value1), b(value2) {
    static_assert(value1 < value2, "Range start must be less than range end");
  }
};

// CTAD for range
template <typename T>
range(T, T) -> range<T>;

// Struct to check if a value is in any of the open intervals
template <typename t, typename... ts>
  requires (tl::all_are_same_v<tl::typelist<ts...>>)
struct is_in_open_range {
  std::array<range<t>, 1 + sizeof...(ts)> open_ranges;

  constexpr is_in_open_range(range<t> range, ::range<ts>... ranges) : open_ranges{range, ranges...} {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const ::range<t>& r1, const ::range<t>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const t& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<t>& r, const t& v) {
      return r.b < v;
    });
    if (it != open_ranges.begin() && (it == open_ranges.end() || it->a > value)) {
      --it;
    }
    return it != open_ranges.end() && it->a < value && value < it->b;
  }
};

// Struct to check if a value is in any of the closed intervals
template <typename T, std::size_t N>
struct is_in_closed_range {
  std::array<range<T>, N> closed_ranges;

  constexpr is_in_closed_range(std::array<range<T>, N> ranges) : closed_ranges(ranges) {
    std::sort(closed_ranges.begin(), closed_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(closed_ranges.begin(), closed_ranges.end(), value, [](const range<T>& r, const T& v) {
      return r.b < v;
    });
    if (it != closed_ranges.begin() && (it == closed_ranges.end() || it->a > value)) {
      --it;
    }
    return it != closed_ranges.end() && it->a <= value && value <= it->b;
  }
};

// CTAD (Class Template Argument Deduction) guides
template <typename T>
eq(T) -> eq<T>;

template <typename T>
neq(T) -> neq<T>;

template <typename T>
lt(T) -> lt<T>;

template <typename T>
gt(T) -> gt<T>;

template <typename T>
lte(T) -> lte<T>;

template <typename T>
gte(T) -> gte<T>;

template <typename t, typename... ts> 
any_of(t, ts...) -> any_of<t, ts...>;

template <typename t, typename... ts> 
is_in_open_range(range<t>, range<ts>...) -> is_in_open_range<t, ts...>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

// #include "field_size.hpp"
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
template <fixed_string id,
          typename T,
          typename runtime_size,
          auto constraint = no_constraint<T>{}> 
struct runtime_field: public basic_field<id, T> {
  static constexpr auto field_accessor = runtime_size::accessor;
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

// What if user wants a custom allocator
template <fixed_string id, typename T, typename runtime_size>
using vec_field = runtime_field<id, std::vector<T>, runtime_size>;

template <fixed_string id, typename runtime_size>
using str_field = runtime_field<id, std::string, runtime_size>;

#endif

struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, std::size_t size>
struct extract_type_from_field<field<id, field_type, field_size<size>>> {
  using type = field_type;
};

template <fixed_string id, typename field_type>
struct extract_type_from_field<struct_field<id, field_type>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;

// todo: How to enable this test
// static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
// static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
// static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
// static_assert(
//   std::is_same_v<extract_type_from_field_v<
//     struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
//     struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
//   );

#endif // _FIELD_META_HPP_

template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, field_containable T, typename size, auto field_constraint>
struct are_all_fields<field_list<field<id, T, size, field_constraint>>> {
  static constexpr bool all_same = true;
};

template <fixed_string id, typename T>
struct are_all_fields<field_list<struct_field<id, T>>> {
  static constexpr bool all_same = true;
};

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <typename T>
struct are_all_fields<T> {
  static constexpr bool all_same = false;
};

template <fixed_string first_id, field_containable T, typename size, auto field_constraint,
          typename... rest>
struct are_all_fields<field_list<field<first_id, T, size, field_constraint>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename... rest>
struct are_all_fields<field_list<struct_field<first_id, T>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename size, auto field_constraint,
          typename... rest>
struct are_all_fields<field_list<runtime_field<first_id, T, size, field_constraint>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

template <typename flist, typename glist>
struct size_indices_resolved;

// if the second list is empty, we have processed all the fields without short circuiting
// so we can treat that we have checked all the fields and all checks have passed
template <typename... xs>
struct size_indices_resolved<
  field_list<xs...>, 
  field_list<>> {
  static constexpr bool is_resolved = true;
};

template <fixed_string id, typename T, std::size_t size, auto constraint, typename... rest>
struct size_indices_resolved<
  field_list<>, 
  field_list<field<id, T, field_size<size>, constraint>, rest...>> {
  static constexpr bool is_resolved = 
    true && 
    size_indices_resolved<
      field_list<field<id, T, field_size<size>, constraint>>, 
      field_list<rest...>
    >::is_resolved;
};

// a struct nested inside a struct shall not have a size dependant on another field, so we can skip
template <fixed_string id, typename T, typename... head, typename... rest>
struct size_indices_resolved<
  field_list<head...>, 
  field_list<struct_field<id, T>, rest...>> {
  static constexpr bool is_resolved = 
    true &&
    size_indices_resolved<
      field_list<head..., struct_field<id, T>>, 
      field_list<rest...>
    >::is_resolved;
};

// if the first field has the size dependant on a field, then its size cannot be deduced
template <fixed_string id, typename T, fixed_string size_source, auto constraint, typename... rest>
struct size_indices_resolved<
  field_list<>, 
  field_list<runtime_field<id, T, runtime_size<from_field<size_source>>, constraint>, rest...>> {
  static constexpr bool is_resolved = false;
};

template <fixed_string id, typename T, std::size_t size, auto constraint, 
          typename x, typename... xs, typename... rest>
struct size_indices_resolved<
  field_list<x, xs...>, 
  field_list<field<id, T, field_size<size>, constraint>, rest...>> {
  static constexpr bool is_resolved =
    true &&
    size_indices_resolved<
      field_list<x, xs..., field<id, T, field_size<size>, constraint>>, 
      field_list<rest...>
    >::is_resolved;
};

template <fixed_string id, typename T, fixed_string size_source, auto constraint, 
          typename x, typename... xs, typename... rest>
struct size_indices_resolved<
  field_list<x, xs...>, 
  field_list<runtime_field<id, T, runtime_size<from_field<size_source>>, constraint>, rest...>> {
  using f = field_lookup_v<field_list<x, xs...>, size_source>;
  static constexpr bool is_resolved =
    !std::is_same_v<f, field_lookup_failed> &&
    // todo ensure that the field type is integral
    // std::is_same_v<extract_type_from_field_v<f>, std::size_t> &&
    size_indices_resolved<
      field_list<x, xs..., field<id, T, runtime_size<from_field<size_source>>, constraint>>, 
      field_list<rest...>
    >::is_resolved;
};

template <typename list>
inline constexpr bool size_indices_resolved_v = size_indices_resolved<field_list<>, list>::is_resolved;

template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  static_assert(are_all_fields_v<field_list<fields...>>, 
                "struct_field_list shall be templated with field like types only");
  static_assert(size_indices_resolved_v<field_list<fields...>>, 
    "sizes not resolved. check if any of the fields which depends on the value of another field, \
     is always to the left of the dependant field and the field it depends on exists ");

  template <typename field_accessor, 
            typename field = field_lookup_v<field_list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr auto& operator[](field_accessor)  {
    return static_cast<field&>(*this).value;
  }

  template <typename field_accessor,
            typename field = field_lookup_v<field_list<fields...>, field_accessor::field_id>>
    requires (!std::is_same_v<field_lookup_failed, field>)
  constexpr const auto& operator[](field_accessor) const {
    return static_cast<const field&>(*this).value;
  }
};

namespace static_test {
static_assert(are_all_fields_v<field_list<field<"a", int, field_size<4u>>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, field_size<4u>>, int>>);
static_assert(!are_all_fields_v<field_list<int, int>>);
static_assert(are_all_fields_v<field_list<>>);
static_assert(are_all_fields_v<field_list<field<"a", int, field_size<4u>>, field<"b", int, field_size<4u>>>>);
static_assert(are_all_fields_v<field_list<field<"a", int, field_size<4u>>, field<"b", fixed_string<4>, field_size<4u>>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, field_size<4u>>, field<"b", fixed_string<4>, field_size<4u>>, float>>);
static_assert(size_indices_resolved_v<field_list<field<"a", int, field_size<4ul>>, field<"b", fixed_string<4>, field_size<4ul>>>>);
static_assert(!size_indices_resolved_v<field_list<runtime_field<"a", int, runtime_size<from_field<"b">>>, field<"b", fixed_string<4>, field_size<4ul>>>>);
static_assert(size_indices_resolved_v<field_list<field<"a", int, field_size<4ul>>, runtime_field<"b", fixed_string<4>, runtime_size<from_field<"a">>>>>);
static_assert(!size_indices_resolved_v<field_list<field<"a", int, field_size<4ul>>, field<"b", fixed_string<4>, field_size<4ul>>, runtime_field<"c", int, runtime_size<from_field<"d">>>>>);
}

#endif // _STRUCT_FIELD_LIST_HPP_

#ifndef _CAST_HPP_
#define _CAST_HPP_

#ifndef _FIELD_META_HPP_
#define _FIELD_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}

#endif // _FIELD_ACCESSOR_HPP_

// todo enforce concept constraints for field_size
template <std::size_t N>
struct field_size {
  static constexpr auto size = N;
};

template <fixed_string id>
using from_field = field_accessor<id>;

template <typename field_accessor>
struct runtime_size {
  // static constexpr auto accessor = field_accessor::field_id;
  static constexpr auto accessor = field_accessor{};
};

#endif // _FIELD_SIZE_HPP_

template <fixed_string id, typename T>
struct field_base {
  static constexpr auto field_id = id;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#include <array>

// Function to void pointer cast
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

// Function to get byte address
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

#endif // _ADDRESS_MANIP_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <concepts>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
struct is_fixed_string;

template <std::size_t N>
struct is_fixed_string<fixed_string<N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_string {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_string_v = is_fixed_string<T>::is_same;

template <typename T>
concept fixed_string_like = is_fixed_string_v<T>;

template <typename T>
struct is_fixed_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_fixed_array<T>::is_same)
struct is_fixed_array<std::array<T, N>> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_fixed_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_fixed_array_v = is_fixed_array<T>::is_same;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (field_list_like<T> || arithmetic<T> || is_c_array<T>::is_same)
struct is_c_array<T[N]> {
  static constexpr bool is_same = true;
};

template <typename T>
struct is_c_array {
  static constexpr bool is_same = false;
};

template <typename T>
inline constexpr bool is_c_array_v = is_c_array<T>::is_same;

// fixed_buffer_like concept
// todo constrain to array of primitives 
// todo check if array of records and arrays are possible for implementation
// todo check if md string is ok
template <typename T>
concept fixed_buffer_like = 
  is_fixed_array_v<T> ||
  is_c_array_v<T> ||
  is_fixed_string_v<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct fixed_string {
  std::array<char, N + 1> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
  constexpr auto size() const { return N; }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N1, std::size_t N2>
constexpr bool operator==(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  if constexpr(N1 != N2) return false;
  return std::string_view{lhs.data()} == std::string_view{rhs.data()};
}

template <std::size_t N1, std::size_t N2>
constexpr bool operator!=(fixed_string<N1> lhs, fixed_string<N2> rhs) {
  return !(lhs == rhs);
}

namespace static_test {
static_assert(fixed_string("hello").size() == 5);
}

#endif // _FIXED_STRING_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

#include <string>
#include <type_traits>

template <typename... ts>
struct field_list{};

namespace typelist {
struct null {};

template <typename... ts>
struct typelist;

template <typename... ts>
struct typelist{};

template <>
struct typelist<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<typelist<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<typelist<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<typelist<u, rest...>, t> { static constexpr bool res = false || any_of<typelist<rest...>, t>::res; };

template <typename typelist, typename type>
inline constexpr bool any_of_v = any_of<typelist, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<typelist<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<typelist<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<typelist<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<typelist<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<typelist<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename tlist>
inline constexpr bool all_are_same_v = all_are_same<tlist>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<typelist<t, ts...>> {
  using front_t = t;
};

template <>
struct front<typelist<>> {
  using front_t = null;
};

template <typename tlist>
using front_t = typename front<tlist>::front_t;

} // namespace typelist

namespace static_tests {
namespace tl = typelist;

static_assert(tl::any_of_v<tl::typelist<int, float, float>, int>);
static_assert(tl::any_of_v<tl::typelist<float, int, float, float>, int>);
static_assert(!tl::any_of_v<tl::typelist<int, int, int>, float>);
static_assert(!tl::any_of_v<tl::typelist<>, float>);

static_assert(tl::all_are_same_v<tl::typelist<int, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, int, float, int, int>>);
static_assert(!tl::all_are_same_v<tl::typelist<int, float, float, int, int>>);
static_assert(tl::all_are_same_v<tl::typelist<int>>);
static_assert(tl::all_are_same_v<tl::typelist<>>);
}

#endif // _TYPELIST_HPP_

namespace tl = typelist;

// Concept for strict callable
template <typename T, typename Arg>
concept strict_callable = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<T, typename std::remove_cvref_t<Arg>>;

// Concepts
template <typename T>
concept equality_comparable = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
    { a != b } -> std::same_as<bool>;
} && !std::is_floating_point_v<T>;

template <typename T>
concept comparable = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
    { a <= b } -> std::same_as<bool>;
    { a >= b } -> std::same_as<bool>;
};

template <typename T>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || is_fixed_string_v<T>);

// Structs for predefined constraints
template <equality_comparable T>
struct eq {
  T v;
  constexpr eq(T value) : v(value) {}
  constexpr eq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v == v; }
};

template <equality_comparable T>
struct neq {
  T v;
  constexpr neq(T value) : v(value) {}
  constexpr neq() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v != v; }
};

template <comparable T>
struct lt {
  T v;
  constexpr lt(T value) : v(value) {}
  constexpr lt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v < v; }
};

template <comparable T>
struct gt {
  T v;
  constexpr gt(T value) : v(value) {}
  constexpr gt() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v > v; }
};

template <typename T>
struct lte {
  T v;
  constexpr lte(T value) : v(value) {}
  constexpr lte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v <= v; }
};

template <inequality_comparable T>
struct gte {
  T v;
  constexpr gte(T value) : v(value) {}
  constexpr gte() : v{} {}
  constexpr bool operator()(const T& actual_v) const { return actual_v >= v; }
};

template <typename T>
struct no_constraint {
  constexpr bool operator()([[maybe_unused]] const T& actual_v) const { 
    return true; 
  }
};

template <typename T, typename... Ts>
  requires (tl::all_are_same_v<tl::typelist<T, Ts...>>)
struct any_of {
  std::array<T, 1 + sizeof...(Ts)> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    return std::find(possible_values.begin(), 
                     possible_values.end(), 
                     actual_v) != possible_values.end();
  }
};

// Range struct
template <typename T>
struct range {
  T a;
  T b;

  constexpr range(T value1, T value2) : a(value1), b(value2) {
    static_assert(value1 < value2, "Range start must be less than range end");
  }
};

// CTAD for range
template <typename T>
range(T, T) -> range<T>;

// Struct to check if a value is in any of the open intervals
template <typename t, typename... ts>
  requires (tl::all_are_same_v<tl::typelist<ts...>>)
struct is_in_open_range {
  std::array<range<t>, 1 + sizeof...(ts)> open_ranges;

  constexpr is_in_open_range(range<t> range, ::range<ts>... ranges) : open_ranges{range, ranges...} {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const ::range<t>& r1, const ::range<t>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const t& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<t>& r, const t& v) {
      return r.b < v;
    });
    if (it != open_ranges.begin() && (it == open_ranges.end() || it->a > value)) {
      --it;
    }
    return it != open_ranges.end() && it->a < value && value < it->b;
  }
};

// Struct to check if a value is in any of the closed intervals
template <typename T, std::size_t N>
struct is_in_closed_range {
  std::array<range<T>, N> closed_ranges;

  constexpr is_in_closed_range(std::array<range<T>, N> ranges) : closed_ranges(ranges) {
    std::sort(closed_ranges.begin(), closed_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(closed_ranges.begin(), closed_ranges.end(), value, [](const range<T>& r, const T& v) {
      return r.b < v;
    });
    if (it != closed_ranges.begin() && (it == closed_ranges.end() || it->a > value)) {
      --it;
    }
    return it != closed_ranges.end() && it->a <= value && value <= it->b;
  }
};

// CTAD (Class Template Argument Deduction) guides
template <typename T>
eq(T) -> eq<T>;

template <typename T>
neq(T) -> neq<T>;

template <typename T>
lt(T) -> lt<T>;

template <typename T>
gt(T) -> gt<T>;

template <typename T>
lte(T) -> lte<T>;

template <typename T>
gte(T) -> gte<T>;

template <typename t, typename... ts> 
any_of(t, ts...) -> any_of<t, ts...>;

template <typename t, typename... ts> 
is_in_open_range(range<t>, range<ts>...) -> is_in_open_range<t, ts...>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

// #include "field_size.hpp"
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
template <fixed_string id,
          typename T,
          typename runtime_size,
          auto constraint = no_constraint<T>{}> 
struct runtime_field: public basic_field<id, T> {
  static constexpr auto field_accessor = runtime_size::accessor;
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

// What if user wants a custom allocator
template <fixed_string id, typename T, typename runtime_size>
using vec_field = runtime_field<id, std::vector<T>, runtime_size>;

template <fixed_string id, typename runtime_size>
using str_field = runtime_field<id, std::string, runtime_size>;

#endif

struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, std::size_t size>
struct extract_type_from_field<field<id, field_type, field_size<size>>> {
  using type = field_type;
};

template <fixed_string id, typename field_type>
struct extract_type_from_field<struct_field<id, field_type>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;

// todo: How to enable this test
// static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
// static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
// static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
// static_assert(
//   std::is_same_v<extract_type_from_field_v<
//     struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
//     struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
//   );

#endif // _FIELD_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint>
struct is_field<field<id, T, field_size, constraint>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_field_with_runtime_size;

template <typename T>
struct is_field_with_runtime_size: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint>
struct is_field_with_runtime_size<runtime_field<id, T, field_size, constraint>>: std::true_type{};

template <typename T>
constexpr bool is_field_with_runtime_size_v = is_field_with_runtime_size<T>::value;

template <typename... fields>
struct struct_field_list;

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, const unsigned char* buffer) {
  std::size_t prefix_sum[sizeof...(fields) + 1] = {0};
  std::size_t index = 0;

  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    prefix_sum[index + 1] = prefix_sum[index] + field_type::field_size;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, buffer + prefix_sum[index]);
    } else if(is_field_v<field_type>) {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_type::field_size);
    } else if(is_field_with_runtime_size_v<field_type>) {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_list[field_type::field_accessor]);
    }
    ++index;
  }(static_cast<fields&>(field_list)), ...);
}

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, std::ifstream& stream) {
  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, stream);
    } else if(is_field_v<field_type>) {
      field.read(stream, field_type::field_size);
    } else if(is_field_with_runtime_size_v<field_type>) {
      field.read(stream, field_list[field_type::field_accessor]);
    }

  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_

#endif // STRUCT_CAST_HPP
