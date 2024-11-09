#ifndef STRUCT_CAST_HPP
#define STRUCT_CAST_HPP

#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#include <array>
#include <string>
#include <vector>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <typename T>
void* to_void_ptr(std::vector<T>& obj) {
  return reinterpret_cast<void*>(obj.data());
}

// todo add overloads for address manip of std::string
// template <>
// void* to_void_ptr(std::string obj) {
//   return reinterpret_cast<void*>(obj.data());
// }

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

template <typename T>
char* byte_addressof(std::vector<T>& obj) {
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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include <functional>
#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;

struct empty_list{};
struct not_a_list{};
struct out_of_bound{};

template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
  static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
  static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
  using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
  using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

template <typename T>
concept field_name_list = is_field_name_list_v<T>;

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

#endif // _FIXED_STR_LIST_HPP_

// todo add constriants
template <auto callable, typename R, field_name_list fstr_list>
struct compute;

template <auto callable, typename R, typename struct_fields, typename field_list, typename indices>
struct is_invocable;

template <auto callable, 
          typename R, 
          typename struct_fields, 
          fixed_string_like req_field_list, 
          std::size_t... idx>
struct is_invocable<callable, 
                    R, 
                    struct_fields, 
                    req_field_list, 
                    std::index_sequence<idx...>> {
  static constexpr bool res = 
      std::is_invocable_r<
        R, 
        decltype(callable),
        decltype(struct_fields{}[field_accessor<front_t<pop_t<idx, req_field_list>>>{}])...
      >::value; 
};

template <auto Func, 
          typename R, 
          typename struct_fields, 
          typename req_fields>
concept can_eval_R_from_fields = 
  is_invocable<
    Func, 
    R, 
    struct_fields, 
    req_fields,
    std::make_integer_sequence<std::size_t, size_v<req_fields>>
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?
template <auto callable, typename R, field_name_list req_fields>
struct compute {
  template <typename... fields, std::size_t... idx>
  constexpr auto invoke_impl(struct_field_list<fields...>& flist, std::index_sequence<idx...>) {
    return std::invoke(callable, flist[field_accessor<front_t<pop_t<idx, req_fields>>>{}]...);
  }

  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                req_fields>)
  constexpr auto operator()(struct_field_list<fields...>& flist) {
    return invoke_impl(flist, std::make_integer_sequence<std::size_t, size_v<req_fields>>{});
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;

template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute<callable, R, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_compute_like_v = is_compute_like<T>::res;

template <typename T>
struct is_eval_bool_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_bool_from_fields<compute<callable, bool, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_bool_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_bool_from_fields_v = is_eval_bool_from_fields<T>::res;

template <typename T>
struct is_eval_size_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_size_from_fields<compute<callable, std::size_t, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;

#endif // _COMPUTE_RES_

#ifndef _CAST_HPP_
#define _CAST_HPP_

#include <expected>
#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_

#include <cstring>
#include <fstream>
#include <expected>
#ifndef _STRUCT_FIELD_LIST_TRAITS_HPP_
#define _STRUCT_FIELD_LIST_TRAITS_HPP_

#include <type_traits>
#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

template <typename... fields>
struct struct_field_list;

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

#endif // _STRUCT_FIELD_LIST_TRAITS_HPP_

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

#ifndef _TYPELIST_MANIP_HPP_
#define _TYPELIST_MANIP_HPP_

#include <variant>
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

#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;

struct empty_list{};
struct not_a_list{};
struct out_of_bound{};

template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
  static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
  static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
  using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
  using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

template <typename T>
concept field_name_list = is_field_name_list_v<T>;

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

#endif // _FIXED_STR_LIST_HPP_

// todo typelist to variant?
template <typename T>
struct variant_from_tlist;

template <typename... type>
struct variant_from_tlist<typelist::typelist<type...>> {
  using variant = std::variant<type...>;
  static constexpr std::size_t N = sizeof...(type);
};

template <typename T>
using variant_from_tlist_v = typename variant_from_tlist<T>::variant;

template <typename T>
inline constexpr std::size_t variant_size_n = variant_from_tlist<T>::N;

static_assert(std::is_same_v<variant_from_tlist_v<typelist::typelist<int, float>>, std::variant<int, float>>);

template <typename T>
struct tl_size;

template <>
struct tl_size<typelist::typelist<>> {
  static constexpr std::size_t N = 0;
};

template <typename head, typename... tail>
struct tl_size<typelist::typelist<head, tail...>> {
  static constexpr std::size_t N = 1 + tl_size<typelist::typelist<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t tl_size_v = tl_size<T>::N;

template <typename T>
struct tl_front;

template <typename head, typename... tail>
struct tl_front<typelist::typelist<head, tail...>> {
  using type = head;
};

template <typename T>
using tl_front_t = typename tl_front<T>::type;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct tl_get;

template <std::size_t key, std::size_t count, typename head, typename... tail>
struct tl_get<key, key, count, typelist::typelist<head, tail...>> {
  using type = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, typename head, typename... tail>
struct tl_get<idx, key, count, typelist::typelist<head, tail...>> {
  using type = typename tl_get<idx + 1, key, count, typelist::typelist<tail...>>::type;
};

template <std::size_t key, typename T>
using tl_get_t = typename tl_get<0, key, tl_size_v<T>, T>::type; 

template <std::size_t count, typename T>
struct tl_pop;

template <std::size_t count>
struct tl_pop<count, typelist::typelist<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, typename head, typename... tail>
struct tl_pop<count, typelist::typelist<head, tail...>> {
  using type = typename pop<count - 1, typelist::typelist<tail...>>::type;
};

template <typename head, typename... tail>
struct tl_pop<0, typelist::typelist<head, tail...>> {
  using type = typelist::typelist<head, tail...>;
};

template <std::size_t count, typename T>
using tl_pop_t = typename tl_pop<count, T>::type;

template <std::size_t idx, typename T>
struct get_nth_variant_type;

template <std::size_t idx, typename... types>
struct get_nth_variant_type<idx, std::variant<types...>> {
  using type = tl_front_t<tl_pop_t<idx, typelist::typelist<types...>>>;
};

template <std::size_t idx, typename T>
using get_nth_variant_type_t = typename get_nth_variant_type<idx, T>::type;

template <typename... types>
struct variant_to_typelist;

template <typename... types>
struct variant_to_typelist<std::variant<types...>> {
  using type = typelist::typelist<types...>;
  static constexpr std::size_t N = sizeof...(types);
};

template <typename T>
using variant_to_typelist_t = typename variant_to_typelist<T>::type;

template <typename T>
inline constexpr std::size_t variant_size = variant_to_typelist<T>::N;

struct out_of_bounds {};

template <typename T>
struct iseq_pop;

template <>
struct iseq_pop<std::integer_sequence<std::size_t>> {
  using type = out_of_bounds;
};

template <std::size_t head, std::size_t... size_tail>
struct iseq_pop<std::integer_sequence<std::size_t, head, size_tail...>> {
  using type = std::integer_sequence<std::size_t, size_tail...>;
};

template <typename T>
using iseq_pop_t = typename iseq_pop<T>::type;

template <typename T>
struct iseq_front;

template <std::size_t head, std::size_t... tail>
struct iseq_front<std::integer_sequence<std::size_t, head, tail...>> {
  static constexpr std::size_t size = head;
};

template <>
struct iseq_front<std::integer_sequence<std::size_t>> {}; //todo: uh?

template <typename T>
inline constexpr std::size_t iseq_front_v = iseq_front<T>::size;

#endif // _TYPELIST_MANIP_HPP_

template <std::size_t idx, typename T, typename U, typename V>
struct variant_reader_impl;

template <std::size_t idx, typename T>
struct variant_reader_impl<idx, T, typelist::typelist<>, std::integer_sequence<std::size_t>> {
  auto operator()([[maybe_unused]] std::size_t idx_r, [[maybe_unused]]std::ifstream& ifs) 
    -> std::expected<std::variant<T>, std::string> {
    std::unexpected("Out of bounds");
  }

  auto operator()([[maybe_unused]] std::size_t idx_r, [[maybe_unused]]const char* buf) 
    -> std::expected<std::variant<T>, std::string> {
    std::unexpected("Out of bounds");
  }
};

template <std::size_t idx, typename T, typename... types, std::size_t... sizes>
struct variant_reader_impl<idx, 
                           T, 
                           typelist::typelist<types...>, 
                           std::integer_sequence<std::size_t, sizes...>> 
{
  using type_to_read = tl_front_t<typelist::typelist<types...>>;
  static constexpr std::size_t size_to_read = iseq_front_v<std::integer_sequence<std::size_t, sizes...>>;

  auto operator()(std::size_t idx_r, std::ifstream& ifs) -> 
    std::expected<std::variant<T>, std::string> {
    if(idx_r == idx) {
      if constexpr(is_struct_field_list_v<type_to_read>) {
        auto res = struct_cast<front_t<typelist::typelist<types...>>>(ifs);
        return (res) ? *res : std::unexpected("error");
      } else {
        type_to_read obj;
        if(!ifs.read(byte_addressof(obj), size_to_read))
          return std::unexpected("buffer exhaustion");
        return obj;
      }
    } else {
      return 
        variant_reader_impl<
         idx + 1, 
         T, 
         tl_pop_t<1, typelist::typelist<types...>>, 
         iseq_pop_t<std::integer_sequence<std::size_t, sizes...>>
       >(idx_r, ifs);
    }
  }

  auto operator()(std::size_t idx_r, const unsigned char* buf) -> 
    std::expected<std::variant<T>, std::string> {
    if(idx_r == idx) {
      if constexpr(is_struct_field_list_v<type_to_read>) {
        auto res = struct_cast<front_t<typelist::typelist<types...>>>(buf);
        return (res) ? *res : std::unexpected("error");
      } else {
        type_to_read obj;
        std::memcpy(to_void_ptr(obj), buf, size_to_read);
      }
    } else {
      return 
        variant_reader_impl<
         idx + 1, 
         T, 
         tl_pop_t<1, typelist::typelist<types...>>, 
         iseq_pop_t<std::integer_sequence<std::size_t, sizes...>>
       >(idx_r, buf);
    }
  }
};

template <typename T, typename U>
struct variant_reader;

template <typename T, std::size_t... sizes>
struct variant_reader<T, std::integer_sequence<std::size_t, sizes...>> {
  using types = variant_to_typelist_t<T>;
  using size_list = std::integer_sequence<std::size_t, sizes...>;
  using read_result = std::expected<T, std::string>;

  auto operator()(std::size_t idx_r, std::istream& ifs) -> read_result {
    variant_reader_impl<0, T, types, size_list>(idx_r, ifs); 
  }

  auto operator()(std::size_t idx_r, const char* buf) -> read_result {
    variant_reader_impl<0, T, types, size_list>(idx_r, buf); 
  }
};

template <typename T>
auto read(const unsigned char* buffer, std::size_t size_to_read) 
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

#endif // _FIELD_READER_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

#ifndef _STRUCT_FIELD_LIST_TRAITS_HPP_
#define _STRUCT_FIELD_LIST_TRAITS_HPP_

#include <type_traits>
#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

template <typename... fields>
struct struct_field_list;

// Metafunction to check if a type is a struct_field_list
template <typename T>
struct is_struct_field_list : std::false_type {};

template <typename... Entries>
struct is_struct_field_list<struct_field_list<Entries...>> : std::true_type {};

template <typename T>
constexpr bool is_struct_field_list_v = is_struct_field_list<T>::value;

#endif // _STRUCT_FIELD_LIST_TRAITS_HPP_

#ifndef _FIELD_META_HPP_
#define _FIELD_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, typename size, auto constraint, auto present_only_if, auto type_deducer>
struct extract_type_from_field<field<id, field_type, field_size<size>, constraint, present_only_if, type_deducer>> {
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

#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

// todo possible dead code
template <typename... expected_types>
auto is_any_error(const expected_types&... expected_list) {
  return (expected_list && ...);
}

// todo constraints
template <typename expected_struct_field_list, typename error>
auto operator|(std::expected<expected_struct_field_list, error>& lhs, auto functor) {
  if(lhs) return functor(*lhs); else return lhs;
}

// todo constraint with struct field list else user might have to provide all
// fields as params to the function
template <typename T>
struct struct_cast_impl;

template <typename... fields>
struct struct_cast_impl<struct_field_list<fields...>> {
  using field_list_type = struct_field_list<fields...>;
  using res_type = std::expected<struct_field_list<fields...>, std::string>;

  constexpr auto operator()(const unsigned char* buffer) -> res_type {
    res_type res;
    std::size_t prefix_sum[sizeof...(fields) + 1] = {0};
    std::size_t index = 0;

    return (
      res | 
      ([&prefix_sum, &buffer, &index](field_list_type input) -> res_type {
        using field_size = typename fields::field_size;
        using field_type = typename fields::field_type;

        auto field = static_cast<fields&>(input);
        std::expected<field_type, std::string> field_value;
        auto buffer_pos = reinterpret_cast<const char*>(buffer + prefix_sum[index]);

        if constexpr (is_optional_field_v<field_type>) {
          // todo variable length optional field
          field_value = field_type::field_presence_checker(input) ? 
                        read<field_type>(buffer_pos, field_type::field_size) :
                        res_type{std::nullopt};
        } else if constexpr (is_union_field_v<field_type>) {
          using variant_reader_type_t = variant_reader<field_type, field_size>;
          auto type_index = field_type::type_deduction_guide(input); 
          field_value = *type_index ? 
                        variant_reader_type_t{}(type_index, buffer_pos): 
                        type_index;
        } else if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
          field_value = struct_cast(field.value, buffer_pos);
        } else if constexpr (is_field_v<field_type>) {
          field_value = read<field_type>(buffer_pos, field_size::size::N);
        } else if constexpr (is_runtime_sized_field_v<field_type>) {
          field_value = read<field_type>(buffer_pos, input[field_type::field_accessor]);
        }

        // todo fix bug in case of updating the prefic sum for var len field
        // also handle optional field, both read and unread case and variant read
        // some func or deduction to get size out of type_size
        prefix_sum[index + 1] = prefix_sum[index] + 0;
        ++index;
        
        // todo return std::unexpected to break the pipeline
        // is this ok?
        if(field_value) field.value = *field_value;
        else input = field_value.error();
        
        // todo constraint checker
        // static constexpr auto constraint_checker = constraint_on_value;
        return input;
      } | 
    ...));
  }
};

template <typename T>
  requires is_struct_field_list_v<T>
constexpr auto struct_cast(const unsigned char* buffer) -> std::expected<T, std::string> {
  return struct_cast_impl<T>{}(buffer);
}

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, std::ifstream& stream) {
  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, stream);
    } else if constexpr (is_field_v<field_type>) {
      field.read(stream, field_type::field_size);
    } else if constexpr (is_runtime_sized_field_v<field_type>) {
      field.read(stream, field_list[field_type::field_accessor]);
    }

  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_

#ifndef _FIELD_TYPE_HPP_
#define _FIELD_TYPE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _TYPE_DEDUCTION_HPP_
#define _TYPE_DEDUCTION_HPP_

#include <variant>
#include <expected>
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

#ifndef _TYPE_LADDER_HPP_
#define _TYPE_LADDER_HPP_

#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include <functional>
#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;

struct empty_list{};
struct not_a_list{};
struct out_of_bound{};

template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
  static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
  static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
  using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
  using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

template <typename T>
concept field_name_list = is_field_name_list_v<T>;

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

#endif // _FIXED_STR_LIST_HPP_

// todo add constriants
template <auto callable, typename R, field_name_list fstr_list>
struct compute;

template <auto callable, typename R, typename struct_fields, typename field_list, typename indices>
struct is_invocable;

template <auto callable, 
          typename R, 
          typename struct_fields, 
          fixed_string_like req_field_list, 
          std::size_t... idx>
struct is_invocable<callable, 
                    R, 
                    struct_fields, 
                    req_field_list, 
                    std::index_sequence<idx...>> {
  static constexpr bool res = 
      std::is_invocable_r<
        R, 
        decltype(callable),
        decltype(struct_fields{}[field_accessor<front_t<pop_t<idx, req_field_list>>>{}])...
      >::value; 
};

template <auto Func, 
          typename R, 
          typename struct_fields, 
          typename req_fields>
concept can_eval_R_from_fields = 
  is_invocable<
    Func, 
    R, 
    struct_fields, 
    req_fields,
    std::make_integer_sequence<std::size_t, size_v<req_fields>>
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?
template <auto callable, typename R, field_name_list req_fields>
struct compute {
  template <typename... fields, std::size_t... idx>
  constexpr auto invoke_impl(struct_field_list<fields...>& flist, std::index_sequence<idx...>) {
    return std::invoke(callable, flist[field_accessor<front_t<pop_t<idx, req_fields>>>{}]...);
  }

  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                req_fields>)
  constexpr auto operator()(struct_field_list<fields...>& flist) {
    return invoke_impl(flist, std::make_integer_sequence<std::size_t, size_v<req_fields>>{});
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;

template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute<callable, R, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_compute_like_v = is_compute_like<T>::res;

template <typename T>
struct is_eval_bool_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_bool_from_fields<compute<callable, bool, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_bool_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_bool_from_fields_v = is_eval_bool_from_fields<T>::res;

template <typename T>
struct is_eval_size_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_size_from_fields<compute<callable, std::size_t, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;

#endif // _COMPUTE_RES_

#include <expected>

// todo return type tag constructed from clause
template <typename... clauses>
struct type_ladder;

template <std::size_t idx, typename... clauses>
struct type_ladder_impl;

template <std::size_t idx>
struct type_ladder_impl<idx> {
  constexpr auto operator()(auto) const -> 
    std::expected<std::size_t, std::string> 
  {
    return std::unexpected("no matches found");
  }
};

// todo constrain clause head and clause_rest
template <std::size_t idx, typename clause_head, typename... clause_rest>
struct type_ladder_impl<idx, clause_head, clause_rest...> {
  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    bool eval_result = clause_head::e(field_list);
    if(eval_result) return idx;
    else return type_ladder_impl<idx + 1, clause_rest...>{}(field_list);
  }
};

template <typename clause_head, typename... clause_rest>
struct type_ladder<clause_head, clause_rest...> {
  // ? is this ok
  using types_only = typelist::typelist<typename clause_head::type_tag::type, typename clause_rest::type_tag::type...>;
  using size_only = std::integer_sequence<std::size_t, clause_head::type_tag::size, clause_rest::type_tag::size...>;

  template <typename... fields>
  constexpr auto operator()(struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, std::string> 
  {
    return type_ladder_impl<0, clause_head, clause_rest...>{}(field_list);
  }
};

#endif // _TYPE_LADDER_HPP_

#ifndef _TYPE_SWITCH_HPP_
#define _TYPE_SWITCH_HPP_

#include <expected>
#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include <functional>
#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;

struct empty_list{};
struct not_a_list{};
struct out_of_bound{};

template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
  static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
  static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
  using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
  using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

template <typename T>
concept field_name_list = is_field_name_list_v<T>;

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

#endif // _FIXED_STR_LIST_HPP_

// todo add constriants
template <auto callable, typename R, field_name_list fstr_list>
struct compute;

template <auto callable, typename R, typename struct_fields, typename field_list, typename indices>
struct is_invocable;

template <auto callable, 
          typename R, 
          typename struct_fields, 
          fixed_string_like req_field_list, 
          std::size_t... idx>
struct is_invocable<callable, 
                    R, 
                    struct_fields, 
                    req_field_list, 
                    std::index_sequence<idx...>> {
  static constexpr bool res = 
      std::is_invocable_r<
        R, 
        decltype(callable),
        decltype(struct_fields{}[field_accessor<front_t<pop_t<idx, req_field_list>>>{}])...
      >::value; 
};

template <auto Func, 
          typename R, 
          typename struct_fields, 
          typename req_fields>
concept can_eval_R_from_fields = 
  is_invocable<
    Func, 
    R, 
    struct_fields, 
    req_fields,
    std::make_integer_sequence<std::size_t, size_v<req_fields>>
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?
template <auto callable, typename R, field_name_list req_fields>
struct compute {
  template <typename... fields, std::size_t... idx>
  constexpr auto invoke_impl(struct_field_list<fields...>& flist, std::index_sequence<idx...>) {
    return std::invoke(callable, flist[field_accessor<front_t<pop_t<idx, req_fields>>>{}]...);
  }

  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                req_fields>)
  constexpr auto operator()(struct_field_list<fields...>& flist) {
    return invoke_impl(flist, std::make_integer_sequence<std::size_t, size_v<req_fields>>{});
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;

template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute<callable, R, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_compute_like_v = is_compute_like<T>::res;

template <typename T>
struct is_eval_bool_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_bool_from_fields<compute<callable, bool, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_bool_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_bool_from_fields_v = is_eval_bool_from_fields<T>::res;

template <typename T>
struct is_eval_size_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_size_from_fields<compute<callable, std::size_t, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;

#endif // _COMPUTE_RES_

#ifndef _MATCH_CASE_HPP_
#define _MATCH_CASE_HPP_

#ifndef _TYPE_TAG_HPP_
#define _TYPE_TAG_HPP_

// todo is this required
// todo constraint T and size
template <typename T, typename size>
struct type_tag {
  using type = T;
  using field_size = size;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<type_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_type_tag_v = is_type_tag<T>::res;

#endif // _TYPE_TAG_HPP_

// todo constrain to data types possible for fields
// todo constrain T?
template <auto v, typename T>
struct match_case {
  static constexpr auto value = v;
  using type_tag = T;
};
template <typename T>
struct is_match_case;

template <auto v, typename h>
struct is_match_case<match_case<v, h>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_match_case {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_match_case_v = is_match_case<T>::res;

template <typename T>
concept match_case_like = is_match_case_v<T>;

#endif // _MATCH_CASE_HPP_

template <std::size_t idx, typename... cases>
struct type_switch_impl;

template <std::size_t idx>
struct type_switch_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, std::string> 
  {
    return std::unexpected("no matches found");
  }
};

// todo check if case and eval result match in terms of types
template <std::size_t idx, match_case_like match_case_head, match_case_like... match_case_rest>
struct type_switch_impl<idx, match_case_head, match_case_rest...> {
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, std::string> 
  {
    if(v == match_case_head::value) return idx;
    else return type_switch_impl<idx + 1, match_case_rest...>{}(v);
  }
};

// atleast one type has to match? but anyways if nothing is matches we get 
// std::unexpected
// todo constrain eval to compute type, cases to match cases
// todo constrain eval return type matches all match case values
// todo return tag constructed with match
template <match_case_like case_head, match_case_like... case_rest>
struct type_switch {
  using types_only = typelist::typelist<typename case_head::type_tag::type, typename case_rest::type_tag::type...>;
  using size_only = std::integer_sequence<std::size_t, case_head::type_tag::size, case_rest::type_tag::size...>;
  
  template <typename... fields>
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, std::string> 
  {
    return type_switch_impl<0, case_head, case_rest...>{}(v);
  } 
};

#endif // _TYPE_SWITCH_HPP_

template <typename... clauses>
struct clauses_to_typelist {
  using tlist = typelist::typelist<typename clauses::type_tag...>;
  // todo aargh, variable length types might have to computed at cast function
};

struct no_type_deduction {};

template <typename T>
struct is_no_type_deduction;

template <typename T>
struct is_no_type_deduction {
  static constexpr bool res = false;
};

template <>
struct is_no_type_deduction<no_type_deduction> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_no_type_deduction_v = is_no_type_deduction<T>::res;

template <typename T>
concept no_type_deduction_like = is_no_type_deduction_v<T>;

template <typename... Args>
struct type;

template <no_type_deduction_like T>
struct type<T> {};

// todo constraints compute like
template <typename eval_expression, typename tswitch>
struct type<eval_expression, tswitch> {
  using expression = eval_expression;
  using type_switch = tswitch;
  using type_selection = tswitch::types_only;
  using size_selection = tswitch::size_only;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, std::string> const {
    type_switch(eval_expression{}(sfl)); 
  }
};

// todo constraints
template <typename tladder>
struct type<tladder> {
  using type_ladder = tladder;
  using type_selection = tladder::types_only;
  using size_selection = tladder::size_only;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, std::string> const {
    return type_ladder(sfl);
  }
};

// todo metafunction and concepts for constraining type

#endif // _TYPE_DEDUCTION_HPP_

#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
#include <functional>
#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_

#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <iostream>
#include <cstddef>  // for std::size_t
#include <climits>  // for CHAR_BIT

class byte_count {
public:
  constexpr explicit byte_count(std::size_t byte_size) 
  : size(byte_size) {}

  constexpr operator std::size_t() const {
    return size * CHAR_BIT; 
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size * CHAR_BIT);
  }

  constexpr byte_count operator+(const byte_count& other) const {
    return byte_count(size + other.size);
  }

  std::size_t size;
};

class bit_count {
public:
  constexpr explicit bit_count(std::size_t bit_size) 
  : size(bit_size) {}

  constexpr bit_count operator+(const bit_count& other) const {
    return bit_count(size + other.size);
  }

  constexpr operator unsigned long long() const {
    return static_cast<unsigned long long>(size);
  }

  std::size_t size;
};

// User-defined literal for byte_count
constexpr byte_count operator"" _B(unsigned long long int value) {
  return byte_count(static_cast<std::size_t>(value));
}

// User-defined literal for bit_count
constexpr bit_count operator"" _b(unsigned long long int value) {
  return bit_count(static_cast<std::size_t>(value));
}

namespace static_test {
  // Using user-defined literals to specify bytes and bits
  // 10 bytes
  constexpr auto byte_count_value = 10_B;
  // 32 bits
  constexpr auto bit_count_value = 32_b;

  static_assert(byte_count_value.size == 10);
  static_assert(bit_count_value.size == 32);

  // Demonstrating conversion from bytes to bits
  constexpr std::size_t bits_from_bytes = static_cast<std::size_t>(byte_count_value);
  static_assert(bits_from_bytes == 80);

  // Adding two bit_count objects
  // Adding 8 bits
  constexpr auto total_bits = bit_count_value + 8_b;
  static_assert(total_bits == 40);

  // Adding two byte_count objects
  // Adding 5 bytes
  constexpr auto total_bytes = byte_count_value + 5_B;
  static_assert(total_bytes.size == 15);

  // Using bit_count with shift operations
  // Shift left by 32 bits
  constexpr unsigned long long bit_shifted_value = 1ULL << static_cast<unsigned long long>(bit_count_value);
  static_assert(bit_shifted_value == 4294967296);

  // Using byte_count with shift operations
  constexpr unsigned long long byte_shifted_value = 1ULL << static_cast<unsigned long long>(9);
  static_assert(byte_shifted_value == 512);
}

#endif // _BYTES_HPP_

template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  static constexpr auto size = size_type{};
};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto size = N;
};

// todo size type for holding multiple sizes in case of union fields
template <std::size_t... N>
struct size_choices;

template <std::size_t... N>
struct size_choices {
  static constexpr auto sizes = std::integer_sequence<std::size_t, N...>{};
};

template <typename T>
struct runtime_size;

template <typename field_accessor>
struct runtime_size {
  static constexpr auto accessor = field_accessor{};
};

template <fixed_string id>
using from_field = runtime_size<field_accessor<id>>;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_comptime_size;

template <std::size_t N>
struct is_comptime_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_size_v = is_comptime_size<T>::res;

template <typename T>
struct is_runtime_size;

template <typename T>
struct is_runtime_size {
  static constexpr bool res = false;
};

template <auto field_accessor>
struct is_runtime_size<field_size<from_field<field_accessor>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_runtime_size_v = is_runtime_size<T>::res;

template <typename T>
struct is_selectable_size;

template <std::size_t... N>
struct is_selectable_size<field_size<size_choices<N...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_comptime_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept comptime_size_like = is_comptime_size_v<T>;

template <typename T>
concept runtime_size_like = is_runtime_size_v<T>;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = comptime_size_like<T> ||
                       runtime_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_runtime_size_v<field_size<from_field<"hello">>>);
static_assert(is_comptime_size_v<field_size<fixed<4>>>);
static_assert(!is_comptime_size_v<int>);
static_assert(!is_runtime_size_v<int>);
}

#endif // _FIELD_SIZE_HPP_

#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

template <typename T>
struct is_field;

template <typename T>
struct is_field: std::false_type{};

template <fixed_string id, typename T, typename field_size, auto constraint, auto presence_check, auto type_deducer>
struct is_field<field<id, T, field_size, constraint, presence_check, type_deducer>>: std::true_type{};

template <typename T>
constexpr bool is_field_v = is_field<T>::value;

template <typename T>
struct is_comptime_sized_field;

template <fixed_string id,
          field_containable T, 
          comptime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_comptime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_comptime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_comptime_sized_field_v = is_comptime_sized_field<T>::res;

template <typename T>
struct is_runtime_sized_field;

// todo: todo var buffer like field constraint
template <fixed_string id,
          typename T, 
          runtime_size_like size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_runtime_sized_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_runtime_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_runtime_sized_field_v = is_runtime_sized_field<T>::res;

template <typename T>
struct is_optional_field;

template <fixed_string id,
          optional_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_optional_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

template <typename T>
struct is_union_field;

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <fixed_string id,
          variant_like T, 
          typename size, 
          auto constraint_on_value, 
          auto present_only_if, 
          auto type_deducer>
struct is_union_field<field<id, T, size, constraint_on_value, present_only_if, type_deducer>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept field_like = is_comptime_sized_field_v<T> ||
                     is_runtime_sized_field_v<T>  ||
                     is_optional_field_v<T>       ||
                     is_union_field_v<T>;

// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }

#endif /*_FIELD_TRAITS_HPP_*/

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

// todo extend for other char types like wchar
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

#ifndef _FIELD__HPP_
#define _FIELD__HPP_

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

#include <vector>
#include <variant>
#include <optional>
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

struct not_a_vec{};

template <typename T>
struct extract_type_from_vec;

template <typename T>
struct extract_type_from_vec<std::vector<T>> {
  using type = T;
};

template <typename T>
struct extract_type_from_vec {
  using type = not_a_vec;
};

template <typename T>
using extract_type_from_vec_t = typename extract_type_from_vec<T>::type;

template <typename T>
struct is_variant_like;

template <typename T>
struct is_variant_like {
  static constexpr bool res = false;
};

template <typename... ts>
struct is_variant_like<std::variant<ts...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variant_like_v = is_variant_like<T>::res;

template <typename T>
concept variant_like = is_variant_like_v<T>;

// todo: add constraints such that user defined optionals can also be used 
// todo: also add constraint to permit var length fields
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

// template <field_containable T>
template <typename T>
struct is_optional_like<std::optional<T>> {
  static inline constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_optional_like_v = is_optional_like<T>::res;

template <typename T>
concept optional_like = is_optional_like_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

#endif // _SC_META_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value,
          auto present_only_if,
          auto type_deducer> 
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  static constexpr auto field_presence_checker = present_only_if;
  static constexpr auto type_deduction_guide = type_deducer;
  field_type value;
};

#endif // _FIELD__HPP_

struct field_lookup_failed {};

template <typename ls, fixed_string id>
struct field_lookup;

template <fixed_string id, 
          typename T, 
          typename size, 
          auto field_constraint, 
          auto present_only_if, 
          auto type_deducer, 
          typename... rest>
struct field_lookup<field_list<field<id, T, size, field_constraint, present_only_if, type_deducer>, rest...>, id> {
  using type = field<id, T, size, field_constraint, present_only_if, type_deducer>;
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

template <typename T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = false;
};

template <field_like T, typename... rest>
struct are_all_fields<field_list<T, rest...>> {
  static constexpr bool all_same = true && are_all_fields<field_list<rest...>>::all_same;
};

template <typename T>
inline constexpr bool are_all_fields_v = are_all_fields<T>::all_same;

// template <are_all_fields... fields>
template <typename... fields>
struct struct_field_list : struct_field_list_base, fields... {
  // static_assert(are_all_fields_v<field_list<fields...>>, 
  //               "struct_field_list shall be templated with field like types only");
  // todo: impl size resolution
  // todo: impl dependencies resolution
  // static_assert(size_indices_resolved_v<field_list<fields...>>, 
  //   "sizes not resolved. check if any of the fields which depends on the value of another field, \
  //    is always to the left of the dependant field and the field it depends on exists ");

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

#endif // _FIELD_LIST__HPP_

#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_

#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_

#include <array>
#include <cstddef>
#include <algorithm>
#include <string_view>

// todo extend for other char types like wchar
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

template <fixed_string... fs>
struct fixed_string_list {};

template <fixed_string... fs>
using with_fields = fixed_string_list<fs...>;

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<with_fields<fs...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_field_name_list_v = is_field_name_list<T>::res;

struct empty_list{};
struct not_a_list{};
struct out_of_bound{};

template <typename T>
struct size;

template <>
struct size<fixed_string_list<>> {
  static constexpr std::size_t N = 0;
};

template <fixed_string head, fixed_string... tail>
struct size<fixed_string_list<head, tail...>> {
  static constexpr std::size_t N = 1 + size<fixed_string_list<tail...>>::N;
};

template <typename T>
inline constexpr std::size_t size_v = size<T>::N;

template <typename T>
struct front;

template <fixed_string head, fixed_string... tail>
struct front<fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <typename T>
inline constexpr auto front_t = front<T>::string;

template <std::size_t idx, std::size_t key, std::size_t count, typename T>
struct get;

template <std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<key, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = head;
};

template <std::size_t idx, std::size_t key, std::size_t count, fixed_string head, fixed_string... tail>
struct get<idx, key, count, fixed_string_list<head, tail...>> {
  static constexpr auto string = get<idx + 1, key, count, fixed_string_list<tail...>>::string;
};

template <std::size_t key, typename T>
inline constexpr auto get_t = get<0, key, size_v<T>, T>::string; 

template <std::size_t count, typename T>
struct pop;

template <std::size_t count>
struct pop<count, fixed_string_list<>> {
  using type = fixed_string_list<>;
};

template <std::size_t count, fixed_string head, fixed_string... tail>
struct pop<count, fixed_string_list<head, tail...>> {
  using type = typename pop<count - 1, fixed_string_list<tail...>>::type;
};

template <fixed_string head, fixed_string... tail>
struct pop<0, fixed_string_list<head, tail...>> {
  using type = fixed_string_list<head, tail...>;
};

template <std::size_t count, typename T>
using pop_t = typename pop<count, T>::type;

template <typename T>
concept field_name_list = is_field_name_list_v<T>;

using typelist_ex = fixed_string_list<"a", "b", "c", "d">;
inline constexpr auto idx_list = std::make_integer_sequence<std::size_t, size_v<typelist_ex>>{};
static_assert(fixed_string("a") == front_t<typelist_ex>);
static_assert(std::is_same_v<fixed_string_list<"a", "b", "c", "d">, pop_t<0, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"b", "c", "d">, pop_t<1, typelist_ex>>);
static_assert(std::is_same_v<fixed_string_list<"c", "d">, pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == front_t<pop_t<2, typelist_ex>>);
static_assert(fixed_string("c") == get_t<2, typelist_ex>);
static_assert(size_v<typelist_ex> == 4);

#endif // _FIXED_STR_LIST_HPP_

// todo add constriants
template <auto callable, typename R, field_name_list fstr_list>
struct compute;

template <auto callable, typename R, typename struct_fields, typename field_list, typename indices>
struct is_invocable;

template <auto callable, 
          typename R, 
          typename struct_fields, 
          fixed_string_like req_field_list, 
          std::size_t... idx>
struct is_invocable<callable, 
                    R, 
                    struct_fields, 
                    req_field_list, 
                    std::index_sequence<idx...>> {
  static constexpr bool res = 
      std::is_invocable_r<
        R, 
        decltype(callable),
        decltype(struct_fields{}[field_accessor<front_t<pop_t<idx, req_field_list>>>{}])...
      >::value; 
};

template <auto Func, 
          typename R, 
          typename struct_fields, 
          typename req_fields>
concept can_eval_R_from_fields = 
  is_invocable<
    Func, 
    R, 
    struct_fields, 
    req_fields,
    std::make_integer_sequence<std::size_t, size_v<req_fields>>
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?
template <auto callable, typename R, field_name_list req_fields>
struct compute {
  template <typename... fields, std::size_t... idx>
  constexpr auto invoke_impl(struct_field_list<fields...>& flist, std::index_sequence<idx...>) {
    return std::invoke(callable, flist[field_accessor<front_t<pop_t<idx, req_fields>>>{}]...);
  }

  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                req_fields>)
  constexpr auto operator()(struct_field_list<fields...>& flist) {
    return invoke_impl(flist, std::make_integer_sequence<std::size_t, size_v<req_fields>>{});
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;

template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute<callable, R, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_compute_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_compute_like_v = is_compute_like<T>::res;

template <typename T>
struct is_eval_bool_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_bool_from_fields<compute<callable, bool, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_bool_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_bool_from_fields_v = is_eval_bool_from_fields<T>::res;

template <typename T>
struct is_eval_size_from_fields;

template <auto callable, fixed_string... req_fields>
struct is_eval_size_from_fields<compute<callable, std::size_t, with_fields<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;

#endif // _COMPUTE_RES_

struct always_true {
  constexpr auto operator()() -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}, with_fields<>>;

// Aliases
// todo enforce constraints wherever applicable
// todo maybe make expected, present_only_if, type_deduce as types
// todo rename expected -> value_constraint

// todo comptime_size_like
template <
  fixed_string id, 
  integral T, 
  comptime_size_like size, 
  auto expected = no_constraint<T>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using basic_field = 
  field<id, T, size, expected, present_only_if, type_deducer>;

template <
  fixed_string id, 
  typename T, 
  std::size_t N, 
  auto expected = no_constraint<std::array<T,N>>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using fixed_array_field = 
  field<
    id, 
    std::array<T, N>, 
    field_size<fixed<N * sizeof(T)>>, 
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  std::size_t N,
  auto expected = no_constraint<fixed_string<N>>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using fixed_string_field = 
  field<
    id, 
    fixed_string<N + 1>, 
    field_size<fixed<N + 1>>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  typename T, 
  std::size_t N,
  auto expected = no_constraint<T[N]>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using c_arr_field = 
  field<
    id, 
    T[N], 
    field_size<fixed<N * sizeof(T)>>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  std::size_t N,
  auto expected = no_constraint<char[N + 1]>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using c_str_field = 
  field<
    id, 
    char[N + 1], 
    field_size<fixed<N * sizeof(char) + 1>>,
    expected, 
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  std::size_t N, 
  std::array<unsigned char, N> expected,
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using magic_byte_array = 
  field<
    id, 
    std::array<unsigned char, N>, 
    field_size<fixed<N>>, 
    eq{expected},
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  fixed_string expected,
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using magic_string = 
  field<
    id, 
    fixed_string<expected.size()>, 
    field_size<fixed<expected.size()>>, 
    eq{expected},
    present_only_if, 
    type_deducer
  >;

template <
  fixed_string id, 
  integral T, 
  comptime_size_like size, 
  T expected,
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using magic_number = 
  field<
    id, 
    T, 
    size, 
    eq{expected},
    present_only_if, 
    type_deducer
  >;

// digressions = What if user wants a custom allocator? use the plain version of the type instead of alias?
// todo get vector length in bytes instead of size to read additional overload
template <
  fixed_string id, 
  typename T, 
  runtime_size_like runtime_size,
  auto expected = no_constraint<T>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using vec_field = 
  field<
    id, 
    std::vector<T>, 
    runtime_size,
    expected,
    present_only_if, 
    type_deducer
>;

// todo check if this will work for all char types like wstring
template <
  fixed_string id, 
  runtime_size_like runtime_size,
  auto expected = no_constraint<std::string>{},
  auto present_only_if = always_present{},
  auto type_deducer = type<no_type_deduction>{}
>
using str_field = 
  field<
    id, 
    std::string, 
    runtime_size,
    expected,
    present_only_if, 
    type_deducer
  >;

// todo: decide if maybe type cannot have type deduction and expected?
template <
  fixed_string id, 
  typename T,
  typename size,
  auto present_only_if,
  auto expected = no_constraint<std::string>{}
>
using maybe_field = 
  field<
    id, 
    std::optional<T>, 
    size,
    expected,
    present_only_if, 
    type<no_type_deduction>{}
  >;

// todo: decide if a variant field be optional
// todo constrains on type_deducer
template <
  fixed_string id, 
  auto type_deducer,
  auto expected = no_constraint<std::string>{}
>
using union_field = 
  field<
    id, 
    typename decltype(type_deducer)::type_selection, 
    typename decltype(type_deducer)::size_selection,
    expected,
    always_present{}, 
    type_deducer
  >;

template <fixed_string id, field_list_like T, auto present_only_if>
using struct_field = 
  field<
    id, 
    T, 
    field_size<fixed<sizeof(T)>>, 
    no_constraint<T>{}, 
    present_only_if, 
    type<no_type_deduction>{}
>;

/*
*   type<
*     eval_result<
*       expression<callable>, 
*       with_fields<...>, 
*     >,
*     type_switch<
*       match_case<.., type_tag_1>,
*       match_case<.., type_tag_2>
*     >
*   >
*   */
/*
*   type<
*     type_switch<
*       eval_result<
*         expression<callable>,
*         with_fields<...>,
*       >, type_tag>,
*       ...
*     >
*   >
*   */

#endif /* _FIELD_TYPE_HPP_ */

#endif // STRUCT_CAST_HPP
