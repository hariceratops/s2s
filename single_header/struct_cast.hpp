#ifndef STRUCT_CAST_HPP
#define STRUCT_CAST_HPP

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

// Concept for strict callable
template <typename T, typename Arg>
concept StrictlyCallableWith = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<Arg, typename std::remove_cvref_t<Arg>>;

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

template <typename T, std::size_t N>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || std::is_same_v<T, fixed_string<N>>);

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

template <typename T>
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
struct any_of {
  static_assert((std::same_as<T, Ts> && ...), "All template parameters must be of the same type");
  std::array<T, sizeof...(Ts) + 1> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    for (const auto& value : possible_values) {
      if (actual_v == value) {
        return true;
      }
    }
    return false;
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
template <typename T, std::size_t N>
struct is_in_open_range {
  std::array<range<T>, N> open_ranges;

  constexpr is_in_open_range(std::array<range<T>, N> ranges) : open_ranges(ranges) {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<T>& r, const T& v) {
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

template <typename T, typename... Ts>
any_of(T, Ts...) -> any_of<T, Ts...>;

template <typename T, std::size_t N>
is_in_open_range(std::array<range<T>, N>) -> is_in_open_range<T, N>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

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
    assert(constraint(value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    ifs.read(byte_addressof(this->value), size_to_read);
    assert(constraint(value));
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

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <fixed_string id, floating_point T>
using float_point_field = field<id, T, sizeof(T)>;

#endif

#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

template <typename... ts>
struct field_list{};

namespace typelist {
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

static_assert(any_of_v<typelist<int, float, float>, int>);
static_assert(any_of_v<typelist<float, int, float, float>, int>);
static_assert(!any_of_v<typelist<int, int, int>, float>);
static_assert(!any_of_v<typelist<>, float>);
} // namespace typelist

#endif // _TYPELIST_HPP_

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _FIELD_LOOKUP_HPP_
#define _FIELD_LOOKUP_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

template <typename... ts>
struct field_list{};

namespace typelist {
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

static_assert(any_of_v<typelist<int, float, float>, int>);
static_assert(any_of_v<typelist<float, int, float, float>, int>);
static_assert(!any_of_v<typelist<int, int, int>, float>);
static_assert(!any_of_v<typelist<>, float>);
} // namespace typelist

#endif // _TYPELIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

// Concept for strict callable
template <typename T, typename Arg>
concept StrictlyCallableWith = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<Arg, typename std::remove_cvref_t<Arg>>;

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

template <typename T, std::size_t N>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || std::is_same_v<T, fixed_string<N>>);

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

template <typename T>
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
struct any_of {
  static_assert((std::same_as<T, Ts> && ...), "All template parameters must be of the same type");
  std::array<T, sizeof...(Ts) + 1> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    for (const auto& value : possible_values) {
      if (actual_v == value) {
        return true;
      }
    }
    return false;
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
template <typename T, std::size_t N>
struct is_in_open_range {
  std::array<range<T>, N> open_ranges;

  constexpr is_in_open_range(std::array<range<T>, N> ranges) : open_ranges(ranges) {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<T>& r, const T& v) {
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

template <typename T, typename... Ts>
any_of(T, Ts...) -> any_of<T, Ts...>;

template <typename T, std::size_t N>
is_in_open_range(std::array<range<T>, N>) -> is_in_open_range<T, N>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

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
    assert(constraint(value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    ifs.read(byte_addressof(this->value), size_to_read);
    assert(constraint(value));
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

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <fixed_string id, floating_point T>
using float_point_field = field<id, T, sizeof(T)>;

#endif

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, typename T, std::size_t size, typename... rest>
struct field_lookup<field_list<field<id, T, size>, rest...>, id> {
  using type = field<id, T, size>;
};

template <fixed_string id, typename T, typename... rest>
struct field_lookup<field_list<struct_field<id, T>, rest...>, id> {
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

template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, field_containable T, std::size_t size>
struct are_all_fields<field_list<field<id, T, size>>> {
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

template <fixed_string first_id, field_containable T, std::size_t size_first,
          typename... rest>
struct are_all_fields<field_list<field<first_id, T, size_first>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename... rest>
struct are_all_fields<field_list<struct_field<first_id, T>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  static_assert(are_all_fields_v<field_list<fields...>>, "struct_field_list shall be templated with field like types only");

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
static_assert(are_all_fields_v<field_list<field<"a", int, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, int>>);
static_assert(!are_all_fields_v<field_list<int, int>>);
static_assert(are_all_fields_v<field_list<>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", int, 4>>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>, float>>);
}

#endif // _STRUCT_FIELD_LIST_HPP_

#ifndef _CAST_HPP_
#define _CAST_HPP_

#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

template <typename... ts>
struct field_list{};

namespace typelist {
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

static_assert(any_of_v<typelist<int, float, float>, int>);
static_assert(any_of_v<typelist<float, int, float, float>, int>);
static_assert(!any_of_v<typelist<int, int, int>, float>);
static_assert(!any_of_v<typelist<>, float>);
} // namespace typelist

#endif // _TYPELIST_HPP_

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
  T value;
};

#endif // _FIELD_BASE_HPP_

#ifndef _FIELD_LOOKUP_HPP_
#define _FIELD_LOOKUP_HPP_

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_

template <typename... ts>
struct field_list{};

namespace typelist {
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

static_assert(any_of_v<typelist<int, float, float>, int>);
static_assert(any_of_v<typelist<float, int, float, float>, int>);
static_assert(!any_of_v<typelist<int, int, int>, float>);
static_assert(!any_of_v<typelist<>, float>);
} // namespace typelist

#endif // _TYPELIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

#ifndef _FIELD_BASE_HPP_
#define _FIELD_BASE_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

template <fixed_string id, typename T, std::size_t size>
struct field_base {
  static constexpr auto field_id = id;
  static constexpr std::size_t field_size = size;
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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

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
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>

// FixedString implementation
template <std::size_t N>
struct fixed_string {
  std::array<char, N> value;
  constexpr fixed_string(): value{} {};
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N, value.data());
  }
  constexpr const char* data() const { return value.data(); }
  constexpr char* data() { return value.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

#endif // _FIXED_STRING_HPP_

// Concept for strict callable
template <typename T, typename Arg>
concept StrictlyCallableWith = requires(T t, Arg arg) {
  { t(arg) } -> std::convertible_to<bool>;
} && std::is_same_v<Arg, typename std::remove_cvref_t<Arg>>;

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

template <typename T, std::size_t N>
concept inequality_comparable = comparable<T> && 
    (std::is_integral_v<T> || std::is_same_v<T, fixed_string<N>>);

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

template <typename T>
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
struct any_of {
  static_assert((std::same_as<T, Ts> && ...), "All template parameters must be of the same type");
  std::array<T, sizeof...(Ts) + 1> possible_values;

  constexpr any_of(T first, Ts... rest) : possible_values{first, rest...} {}

  constexpr bool operator()(const T& actual_v) const {
    for (const auto& value : possible_values) {
      if (actual_v == value) {
        return true;
      }
    }
    return false;
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
template <typename T, std::size_t N>
struct is_in_open_range {
  std::array<range<T>, N> open_ranges;

  constexpr is_in_open_range(std::array<range<T>, N> ranges) : open_ranges(ranges) {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const range<T>& r1, const range<T>& r2) {
      return r1.a < r2.a;
    });
  }

  constexpr bool operator()(const T& value) const {
    auto it = std::lower_bound(open_ranges.begin(), open_ranges.end(), value, [](const range<T>& r, const T& v) {
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

template <typename T, typename... Ts>
any_of(T, Ts...) -> any_of<T, Ts...>;

template <typename T, std::size_t N>
is_in_open_range(std::array<range<T>, N>) -> is_in_open_range<T, N>;

template <typename T, std::size_t N>
is_in_closed_range(std::array<range<T>, N>) -> is_in_closed_range<T, N>;

#endif // FIELD_CONSTRAINT_HPP

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
    assert(constraint(value));
  }

  void read(std::ifstream& ifs, std::size_t size_to_read) {
    ifs.read(byte_addressof(this->value), size_to_read);
    assert(constraint(value));
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

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <fixed_string id, floating_point T>
using float_point_field = field<id, T, sizeof(T)>;

#endif

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, typename T, std::size_t size, typename... rest>
struct field_lookup<field_list<field<id, T, size>, rest...>, id> {
  using type = field<id, T, size>;
};

template <fixed_string id, typename T, typename... rest>
struct field_lookup<field_list<struct_field<id, T>, rest...>, id> {
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

template <typename... ts>
struct are_all_fields;

template <>
struct are_all_fields<field_list<>> { static constexpr bool all_same = true; };

template <fixed_string id, field_containable T, std::size_t size>
struct are_all_fields<field_list<field<id, T, size>>> {
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

template <fixed_string first_id, field_containable T, std::size_t size_first,
          typename... rest>
struct are_all_fields<field_list<field<first_id, T, size_first>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <fixed_string first_id, typename T, typename... rest>
struct are_all_fields<field_list<struct_field<first_id, T>, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  static_assert(are_all_fields_v<field_list<fields...>>, "struct_field_list shall be templated with field like types only");

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
static_assert(are_all_fields_v<field_list<field<"a", int, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, int>>);
static_assert(!are_all_fields_v<field_list<int, int>>);
static_assert(are_all_fields_v<field_list<>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", int, 4>>>);
static_assert(are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>>>);
static_assert(!are_all_fields_v<field_list<field<"a", int, 4>, field<"b", fixed_string<4>, 4>, float>>);
}

#endif // _STRUCT_FIELD_LIST_HPP_

struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, std::size_t size>
struct extract_type_from_field<field<id, field_type, size>> {
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

static_assert(std::is_same_v<extract_type_from_field_v<field<"x", int, 4>>, int>);
static_assert(std::is_same_v<extract_type_from_field_v<field<"x", float, 4>>, float>);
static_assert(std::is_same_v<extract_type_from_field_v<std::array<char, 10>>, not_a_field>);
static_assert(
  std::is_same_v<extract_type_from_field_v<
    struct_field<"d", struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>>, 
    struct_field_list<field<"x", int, 4>, field<"y", int, 4>>>
  );

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
    } else {
      field.read(reinterpret_cast<const char*>(buffer + prefix_sum[index]), field_type::field_size);
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
    } else {
      field.read(stream, field_type::field_size);
    }
  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_

#endif // STRUCT_CAST_HPP
