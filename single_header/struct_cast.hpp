
// Begin /home/hari/Code/struct_cast/worktree/memstream/include/fixed_string.hpp
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/fixed_string.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/address_manip.hpp
#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_

#include <array>
#include <string>
#include <vector>
 
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

template <typename T>
void* to_void_ptr(std::string& obj) {
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

inline char* byte_addressof(std::string& obj) {
  return reinterpret_cast<char*>(&obj[0]);
}

#endif // _ADDRESS_MANIP_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/address_manip.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_accessor.hpp
#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_
 
template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};

template <fixed_string id>
constexpr auto operator""_f() {
  return field_accessor<id>{};
}


#endif // _FIELD_ACCESSOR_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_accessor.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/fixed_str_list.hpp
#ifndef _FIXED_STR_LIST_HPP_
#define _FIXED_STR_LIST_HPP_
 
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/fixed_str_list.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/typelist.hpp
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/typelist.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_size.hpp
#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_
 
 
 
template <typename size_type>
struct field_size;

template <typename size_type>
struct field_size {
  using size_type_t = size_type;
  // static constexpr auto size = size_type{};
};

struct size_dont_care {};

template <std::size_t N>
struct fixed;

template <std::size_t N>
struct fixed {
  static constexpr auto count = N;
};

template <fixed_string id>
using len_from_field = field_accessor<id>;

template <auto callable, field_name_list req_fields>
struct size_from_fields;

// todo constraint for callable
template <auto callable, field_name_list req_fields>
struct size_from_fields {
  static constexpr auto f = callable;
  static constexpr auto req_field_list = req_fields{};
};

template <auto callable, field_name_list ids>
using len_from_fields = size_from_fields<callable, ids>;

// todo size type for holding multiple sizes in case of union fields
template <typename... size_type>
struct size_choices;

template <typename... size_type>
struct size_choices {
  using choices = typelist::typelist<size_type...>;
  static auto constexpr num_of_choices = sizeof...(size_type);
};

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_fixed_size;

template <std::size_t N>
struct is_fixed_size<field_size<fixed<N>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_fixed_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_fixed_size_v = is_fixed_size<T>::res;

template <typename T>
struct is_variable_size;

template <typename T>
struct is_variable_size {
  static constexpr bool res = false;
};

template <fixed_string id>
struct is_variable_size<field_size<len_from_field<id>>> {
  static constexpr bool res = true;
};

template <auto callable, field_name_list ids>
struct is_variable_size<field_size<len_from_fields<callable, ids>>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variable_size_v = is_variable_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept fixed_size_like = is_fixed_size_v<T>;

template <typename T>
concept variable_size_like = is_variable_size_v<T>;

template <typename T>
struct is_selectable_size;

template <typename T>
concept atomic_size = fixed_size_like<T> || variable_size_like<T>;

template <atomic_size... size_type>
struct is_selectable_size<field_size<size_choices<size_type...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_fixed_size<T>::res;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = fixed_size_like<T>    ||
                       variable_size_like<T> ||
                       selectable_size_like<T>;

namespace static_test {
static_assert(is_variable_size_v<field_size<len_from_field<"hello">>>);
static_assert(is_fixed_size_v<field_size<fixed<4>>>);
static_assert(!is_fixed_size_v<int>);
static_assert(!is_variable_size_v<int>);
static_assert(field_size<fixed<6>>::size_type_t::count == 6);
}

#endif // _FIELD_SIZE_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_size.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/struct_field_list_base.hpp
#ifndef _STRUCT_FIELD_LIST_BASE_HPP_
#define _STRUCT_FIELD_LIST_BASE_HPP_

#include <type_traits>

struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;

#endif // _STRUCT_FIELD_LIST_BASE_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/struct_field_list_base.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/sc_type_traits.hpp
#ifndef _SC_META_HPP_
#define _SC_META_HPP_

#include <vector>
#include <variant>
#include <optional>
 
 
// Arithmetic concept
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept trivial = floating_point<T> || integral<T>;

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
  requires (arithmetic<T> || is_fixed_array<T>::is_same)
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
concept fixed_array_like = is_fixed_array_v<T>;

template <typename T>
struct is_c_array;

template <typename T, std::size_t N>
  requires (arithmetic<T> || is_c_array<T>::is_same)
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
struct is_vector_like;

// vector of vectors or vector of arrays?
template <typename T>
  requires (arithmetic<T> || is_fixed_array<T>::is_same)
struct is_vector_like<std::vector<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_vector_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_vector_v = is_vector_like<T>::res;

template <typename T>
concept vector_like = is_vector_v<T>;

template <typename T>
struct is_string_like;

// vector of vectors or vector of arrays?
template <>
struct is_string_like<std::string> {
  static constexpr bool res = true;
};

template <typename T>
struct is_string_like {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_string_v = is_string_like<T>::res;

template <typename T>
concept string_like = is_string_v<T>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

template <typename T>
struct is_array_of_records;

template <field_list_like T, std::size_t N>
struct is_array_of_records<std::array<T, N>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_array_of_records {
  static constexpr bool res = false;
};

template <typename T>
constexpr inline bool is_array_of_records_v = is_array_of_records<T>::res;

template <typename T>
concept array_of_records_like = is_array_of_records_v<T>;

template <typename T>
struct is_vector_of_records;

template <field_list_like T>
struct is_vector_of_records<std::vector<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_vector_of_records {
  static constexpr bool res = false;
};

template <typename T>
constexpr inline bool is_vector_of_records_v = is_vector_of_records<T>::res;

template <typename T>
concept vector_of_records_like = is_vector_of_records_v<T>;

struct not_an_array {};

template <typename T>
struct extract_type_from_array;

template <typename T, std::size_t N>
struct extract_type_from_array<std::array<T, N>> {
  using type = T;
};

template <typename T>
struct extract_type_from_array {
  using type = not_an_array;
};

template <typename T>
using extract_type_from_array_v = extract_type_from_array<T>::type;

template <typename T>
struct extract_size_from_array;

template <typename T, std::size_t N>
struct extract_size_from_array<std::array<T, N>> {
  static constexpr auto size = N;
};

template <typename T>
inline constexpr std::size_t extract_size_from_array_v = extract_size_from_array<T>::size;

template <typename T>
concept variable_sized_buffer_like = vector_like<T> || string_like<T>;

#endif // _SC_META_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/sc_type_traits.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_constraints.hpp
#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
 
 
 
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_constraints.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field.hpp
#ifndef _FIELD__HPP_
#define _FIELD__HPP_
 
 
 
#include <type_traits>

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value>
struct field {
  using field_type = T;
  using field_size = size_type;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  field_type value;
};


template <typename T>
struct to_optional_field;

template <fixed_string id, typename T, typename size_type, auto constraint_on_value>
struct to_optional_field<field<id, T, size_type, constraint_on_value>> {
  using res = field<id, std::optional<T>, size_type, constraint_on_value>;
};

template <typename T>
using to_optional_field_v = to_optional_field<T>::res;

template <typename T>
struct no_variance_field;

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value>
struct no_variance_field<field<id, T, size_type, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct no_variance_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool no_variance_field_v = no_variance_field<T>::res;

template <typename T>
concept no_variance_field_like = no_variance_field_v<T>;

// todo maybe provide field directly as template parameter constrained?
// will solve constraint issue and also produces clean API
template <no_variance_field_like base_field,
          typename present_only_if,
          typename optional = to_optional_field_v<base_field>>
class maybe_field : public optional
{
public:
  using field_base_type = base_field;
  using field_presence_checker = present_only_if;
};


template <typename... choices>
struct field_choice_list {};

template <fixed_string id, typename... args>
struct to_field_choices;

template <fixed_string id, typename T, typename field_size>
struct to_field_choice {
  using field_choice = field<id, T, field_size, no_constraint<T>{}>;
};

template <fixed_string id, typename T, typename field_size>
using to_field_choice_v = to_field_choice<id, T, field_size>::field_choice;

template <fixed_string id, typename... types, typename... sizes>
struct to_field_choices<id, std::variant<types...>, field_size<size_choices<sizes...>>> {
  using choices = field_choice_list<to_field_choice_v<id, types, sizes>...>;
};


template <typename arg>
struct are_unique_types;

template <typename head>
struct are_unique_types<field_choice_list<head>> {
  static constexpr bool res = true;
};

template <typename head, typename neck, typename... tail>
struct are_unique_types<field_choice_list<head, neck, tail...>> {
  constexpr static bool res =
    (!std::is_same_v<head, neck> && ... && (!std::is_same_v<head, tail>)) &&
    are_unique_types<field_choice_list<neck, tail...>>::res;
};


template <typename choice_list>
inline constexpr bool are_unique_types_v = are_unique_types<choice_list>::res;

static_assert(!are_unique_types_v<field_choice_list<int, int, float>>);
static_assert(are_unique_types_v<field_choice_list<int, double, float>>);
static_assert(are_unique_types_v<field_choice_list<int, std::vector<double>, std::vector<float>>>);


// todo how to handle constraint_on_value in general
template <fixed_string id,
          typename type_deducer,
          typename type = typename type_deducer::variant,
          typename size_type = typename type_deducer::sizes,
          auto constraint_on_value = no_constraint<type>{},
          typename variant = field<id, type, size_type, constraint_on_value>,
          typename field_choices_t = to_field_choices<id, type, size_type>::choices
  >
  requires are_unique_types_v<field_choices_t>
struct union_field: public variant {
  using type_deduction_guide = type_deducer;
  using field_choices = field_choices_t;
  static constexpr auto variant_size = std::variant_size_v<type>;
};


#endif // _FIELD__HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/field.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_meta.hpp
#ifndef _FIELD_META_HPP_
#define _FIELD_META_HPP_
 
 
 
struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, typename size, auto constraint>
struct extract_type_from_field<field<id, field_type, field_size<size>, constraint>> {
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_meta.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_traits.hpp
#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_
 
 
 
template <typename T>
struct is_fixed_sized_field;

// Specialization for field with fixed_size_like size
template <fixed_string id, field_containable T, fixed_size_like size, auto constraint_on_value>
struct is_fixed_sized_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_fixed_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_fixed_sized_field_v = is_fixed_sized_field<T>::res;

template <typename T>
concept fixed_sized_field_like = is_fixed_sized_field_v<T>;

template <typename T>
struct is_array_of_record_field;

template <fixed_string id, field_list_like T, std::size_t N, typename size, auto constraint_on_value>
struct is_array_of_record_field<field<id, std::array<T, N>, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_array_of_record_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_array_of_record_field_v = is_array_of_record_field<T>::res;

template <typename T>
concept array_of_record_field_like = is_array_of_record_field_v<T>;


template <typename T>
struct is_variable_sized_field;

// Specialization for field with variable_size_like size
template <fixed_string id, variable_sized_buffer_like T, variable_size_like size, auto constraint_on_value>
struct is_variable_sized_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_variable_sized_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_variable_sized_field_v = is_variable_sized_field<T>::res;

// Concept for variable_sized_field_like
template <typename T>
concept variable_sized_field_like = is_variable_sized_field_v<T>;

template <typename T>
struct is_vector_of_record_field;

template <fixed_string id, field_list_like T, typename size, auto constraint_on_value>
struct is_vector_of_record_field<field<id, std::vector<T>, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_vector_of_record_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_vector_of_record_field_v = is_vector_of_record_field<T>::res;

template <typename T>
concept vector_of_record_field_like = is_vector_of_record_field_v<T>;

template <typename T>
struct is_struct_field;

// Specialization for field with variable_size_like size
template <fixed_string id, field_list_like T, typename size, auto constraint_on_value>
struct is_struct_field<field<id, T, size, constraint_on_value>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_struct_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_struct_field_v = is_struct_field<T>::res;

// Concept for variable_sized_field_like
template <typename T>
concept struct_field_like = is_struct_field_v<T>;


template <typename T>
struct is_optional_field;

// Specialization for maybe_field with a field
template <fixed_string id, 
          typename T, 
          typename size, 
          auto constraint, 
          typename present_only_if, 
          typename optional>
struct is_optional_field<
    maybe_field<field<id, T, size, constraint>, present_only_if, optional>
  >
{
  static constexpr bool res = true;
};

template <typename T>
struct is_optional_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_optional_field_v = is_optional_field<T>::res;

// Concept for optional_field_like
template <typename T>
concept optional_field_like = is_optional_field_v<T>;


template <typename T>
struct is_union_field;

template <fixed_string id,
          typename type_deducer,
          typename type,
          typename size_type,
          auto constraint_on_value,
          typename variant,
          typename field_choices_t>
struct is_union_field<
    union_field<id, type_deducer, type, size_type, constraint_on_value, variant, field_choices_t>
  > 
{
  static constexpr bool res = true;
};

template <typename T>
struct is_union_field {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_union_field_v = is_union_field<T>::res;

template <typename T>
concept union_field_like = is_union_field_v<T>;

// todo struct_field_like

template <typename T>
concept field_like = fixed_sized_field_like<T> || 
                     variable_sized_field_like<T> ||
                     array_of_record_field_like<T> ||
                     vector_of_record_field_like<T> ||
                     struct_field_like<T> || 
                     optional_field_like<T> || 
                     union_field_like<T>;
//
// namespace static_test {
//   static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
//   static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
// }
#endif /*_FIELD_TRAITS_HPP_*/

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_traits.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_lookup.hpp
#ifndef _FIELD_LOOKUP_HPP_
#define _FIELD_LOOKUP_HPP_
 
 
 
// Sentinel type for a failed lookup
struct field_lookup_failed {};

// Primary template declaration for field_lookup
template <typename FieldList, fixed_string Id>
struct field_lookup;

// Success case 1: Match a field with the given id
template <fixed_string id, 
          typename T, 
          typename size_type, 
          auto constraint, 
          typename... rest>
struct field_lookup<
    field_list<field<id, T, size_type, constraint>, rest...>, id
  >
{
  using type = field<id, T, size_type, constraint>;
};

// Success case 2: Match a maybe_field with the given id
template <fixed_string id, 
          typename T, 
          typename size, 
          typename present_only_if, 
          auto constraint, 
          typename optional,
          typename... rest>
struct field_lookup<
    field_list<maybe_field<field<id, T, size, constraint>, present_only_if, optional>, rest...>, id
  > 
{
  using type = 
    maybe_field<
      field<id, T, size, constraint>,
      present_only_if, 
      optional
    >;
};

// Success case 3: Match a union_field with the given id
template <fixed_string id,
          typename type_deducer,
          auto constraint_on_value,
          typename T,
          typename size_type,
          typename variant,
          typename field_choices_t,
          typename... rest>
struct field_lookup<
    field_list<union_field<id, type_deducer, T, size_type, constraint_on_value, variant, field_choices_t>, rest...>, id
  >
{
  using type = union_field<id, type_deducer, T, size_type, constraint_on_value, variant, field_choices_t>;
};

// Recursive case: id does not match the head, continue searching in the rest
template <typename head, typename... rest, fixed_string id>
struct field_lookup<field_list<head, rest...>, id> {
  using type = typename field_lookup<field_list<rest...>, id>::type;
};

// Failure case: Reached the end of the field list without finding a match
template <fixed_string id>
struct field_lookup<field_list<>, id> {
  using type = field_lookup_failed;
};

// Alias for easier use
template <typename field_list_t, fixed_string id>
using field_lookup_v = typename field_lookup<field_list_t, id>::type;

#endif // _FIELD_LOOKUP_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_lookup.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_list.hpp
#ifndef _FIELD_LIST__HPP_
#define _FIELD_LIST__HPP_
 
 
template <fixed_string... arg>
struct are_unique_fixed_strings;

template <fixed_string head>
struct are_unique_fixed_strings<head> {
  static constexpr bool res = true;
};

template <fixed_string head, fixed_string neck, fixed_string... tail>
struct are_unique_fixed_strings<head, neck, tail...> {
  constexpr static bool res =
    ((head != neck) && ... && (head != tail)) &&
    are_unique_fixed_strings<neck, tail...>::res;
};


template <fixed_string... field_ids>
inline constexpr bool has_unique_field_ids_v = are_unique_fixed_strings<field_ids...>::res;

static_assert(!has_unique_field_ids_v<"hello", "world", "hello">);
static_assert(has_unique_field_ids_v<"hello", "world", "nexus">);


template <typename... fields>
concept all_field_like = (field_like<fields> && ...);

// todo: impl size resolution size_indices_resolved_v<field_list<fields...>>
// todo: impl dependencies resolution
template <typename... fields>
  requires all_field_like<fields...> &&
           has_unique_field_ids_v<fields::field_id...>
struct struct_field_list : struct_field_list_base, fields... {
  struct_field_list() = default;
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_list.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/compute_res.hpp
#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include <type_traits>
 
 
 
// todo add constriants
// template <auto callable, typename R, field_name_list fstr_list>
// struct compute;


template <auto callable, typename return_type, typename struct_field_list_t, field_name_list field_list>
struct is_invocable;

template <auto callable, 
          typename return_type, 
          typename struct_field_list_t, 
          fixed_string... req_fields>
struct is_invocable<callable, 
                    return_type, 
                    struct_field_list_t, 
                    fixed_string_list<req_fields...>> {
  static constexpr bool res = 
      std::is_invocable_r_v<
        return_type, 
        decltype(callable),
        decltype(struct_field_list_t{}[field_accessor<req_fields>{}])...
      >;
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
    req_fields
  >::res;

// todo: expression evaluation requested by user shall not be empty but default to empty by library
// todo bring invocable compatibility at type level for strong type guarantee
// todo simplified concept or requires clause
// todo should cv qualification be removed
// todo role of with_fields and variadic arguments must be reversed, can typelist + idx be used?

template <auto callable, typename R, field_name_list Fs>
struct compute;

template <auto callable, typename R, fixed_string... req_fields>
struct compute<callable, R, fixed_string_list<req_fields...>>{
  template <typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list<fields...>,
                fixed_string_list<req_fields...>>)
  constexpr auto operator()(const struct_field_list<fields...>& flist) const -> R {
    return callable(flist[field_accessor<req_fields>{}]...);
  }
};

template <auto callable, field_name_list req_fields>
using eval_bool_from_fields = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using predicate = compute<callable, bool, req_fields>;
template <auto callable, field_name_list req_fields>
using eval_size_from_fields = compute<callable, std::size_t, req_fields>;
template <auto callable, field_name_list req_fields>
using parse_if = eval_bool_from_fields<callable, req_fields>;

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

// End /home/hari/Code/struct_cast/worktree/memstream/include/compute_res.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/size_deduce.hpp
#ifndef _SIZE_DEDUCE_HPP_
#define _SIZE_DEDUCE_HPP_
 
 
 
#include <utility>

template <typename T>
struct deduce_field_size;

template <std::size_t N>
struct deduce_field_size<field_size<fixed<N>>> {
  using field_size_type = field_size<fixed<N>>;

  constexpr auto operator()() -> std::size_t {
    return field_size_type::size_type_t::count;
  }
};

// Can work with from_field since from_field is an alias for runtime_size with single field dependency
// template <fixed_string id>
// using from_field = runtime_size<field_accessor<id>>;
template <fixed_string id>
struct deduce_field_size<field_size<field_accessor<id>>> {
  using field_size_type = field_accessor<id>;
  
  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& struct_fields) -> std::size_t {
    return struct_fields[field_size_type{}];
  }
};

template <auto callable, field_name_list req_fields>
struct deduce_field_size<field_size<size_from_fields<callable, req_fields>>> {
  using field_size_type = compute<callable, std::size_t, req_fields>;
  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& struct_fields) -> std::size_t {
    return field_size_type{}(struct_fields);
  }
};

// todo: decide on upcounting vs downcounting for this mess
template <std::size_t size_idx, typename... sizes>
struct deduce_field_size_switch;

template <std::size_t size_idx>
struct deduce_field_size_switch<size_idx, field_size<size_choices<>>> {
  template <typename... fields>
  constexpr auto operator()(std::size_t, const struct_field_list<fields...>&) -> std::size_t {
    std::unreachable();
  }
};

template <std::size_t size_idx, typename head, typename... tail>
struct deduce_field_size_switch<size_idx, field_size<size_choices<head, tail...>>> {
  template <typename... fields>
  constexpr auto operator()(std::size_t size_idx_r, const struct_field_list<fields...>& struct_fields) -> std::size_t {
    if(size_idx_r == size_idx) {
      if constexpr(fixed_size_like<head>) return deduce_field_size<head>{}();
      else return deduce_field_size<head>{}(struct_fields);
    } else {
      return deduce_field_size_switch<size_idx + 1, field_size<size_choices<tail...>>>{}(size_idx_r, struct_fields);
    } 
  }
};

// todo meta function for size_choice count
// template <atomic_size... sizes>
template <typename... sizes>
struct deduce_field_size<field_size<size_choices<sizes...>>> {
  constexpr static auto num_of_choices = sizeof...(sizes);

  template <typename... fields>
  constexpr auto operator()(std::size_t size_idx_r, const struct_field_list<fields...>& struct_fields) -> std::size_t {
    return deduce_field_size_switch<0, field_size<size_choices<sizes...>>>{}(size_idx_r, struct_fields);
  }
};

#endif // _SIZE_DEDUCE_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/size_deduce.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/error.hpp
#ifndef _ERROR_HPP_
#define _ERROR_HPP_

#include <expected>

enum cast_error {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure
};


using read_result = std::expected<void, cast_error>;

#endif // _ERROR_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/error.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/pipeline.hpp
#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_

#include <expected>
 
// todo possible dead code
template <typename... expected_types>
auto is_any_error(const expected_types&... expected_list) {
  return (expected_list && ...);
}

// todo constraints
// todo fix the copying by using reference currently error thrown due to 
// non const lvalue being bound to rvalue
auto operator|(const read_result& res, auto&& callable) -> read_result
{
  return res ? callable() : std::unexpected(res.error());
}

// auto operator|(auto&& callable_left, auto&& callable_right) -> read_result  {
//   return callable_left() | callable_right;
// }

#endif // _PIPELINE_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/pipeline.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_reader.hpp
#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_

#include <cstring>
#include <print>
#include <fstream>
#include <expected>
#include <utility>
 
 
 
 
  
template <typename T>
// todo specialise for non scalar type to facilitate endianness specific vector read
constexpr auto raw_read(T& value, std::ifstream& ifs, std::size_t size_to_read) 
  -> read_result 
{
  if(!ifs.read(byte_addressof(value), size_to_read))
    return std::unexpected(cast_error::buffer_exhaustion);
  return {};
}


template <typename T>
constexpr auto read_scalar(T& value, std::size_t size_to_read, std::ifstream& ifs)
  -> read_result
{
  return raw_read(value, ifs, size_to_read);
}


template <typename T>
constexpr auto read_vector(T& value, std::size_t len_to_read, std::ifstream& ifs) 
  -> read_result 
{
  constexpr auto size_of_one_elem = sizeof(T{}[0]);
  value.resize(len_to_read);
  return raw_read(value, ifs, len_to_read * size_of_one_elem);
}


// todo inheritance for ctor boilerplate removal: read<t,f>?
template <typename F, typename L>
struct read_field;

template <fixed_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using field_size = typename T::field_size;
    constexpr auto size_to_read = deduce_field_size<field_size>{}();
    return read_scalar(field.value, size_to_read, ifs);
  }
};


template <variable_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using field_size = typename T::field_size;
    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    return read_vector(field.value, len_to_read, ifs);
  }
};


struct not_array_of_records_field {};

template <typename T>
struct create_field_from_array_of_records;

template <array_of_record_field_like T>
struct create_field_from_array_of_records<T> {
  using array_type = typename T::field_type;
  using array_elem_type = extract_type_from_array_v<array_type>;
  static constexpr auto field_id = T::field_id;
  using size = field_size<size_dont_care>;
  static constexpr auto constraint = no_constraint<array_elem_type>{};

  using res = field<field_id, array_elem_type, size, constraint>;
};

template <typename T>
using create_field_from_array_of_records_v = create_field_from_array_of_records<T>::res;

struct not_vector_of_records_field {};

template <typename T>
struct create_field_from_vector_of_records;

template <vector_of_record_field_like T>
struct create_field_from_vector_of_records<T> {
  using vector_type = typename T::field_type;
  using vector_elem_type = extract_type_from_vec_t<vector_type>;
  static constexpr auto field_id = T::field_id;
  using size = field_size<size_dont_care>;
  static constexpr auto constraint = no_constraint<vector_elem_type>{};

  using res = field<field_id, vector_elem_type, size, constraint>;
};

template <typename T>
using create_field_from_vector_of_records_v = create_field_from_vector_of_records<T>::res;

template <typename T, typename F, typename E>
struct read_buffer_of_records {
  T& field;
  F& field_list;
  std::ifstream& ifs;
  std::size_t len_to_read;

  constexpr read_buffer_of_records(T& field, F& field_list, std::ifstream& ifs, std::size_t len_to_read)
    : field(field), field_list(field_list), ifs(ifs), len_to_read(len_to_read) {}

  constexpr auto operator()() const -> read_result {
    for(std::size_t count = 0; count < len_to_read; ++count) {
      E elem;
      auto reader = read_field<E, F>(elem, field_list, ifs);
      auto res = reader();
      if(!res) 
        return std::unexpected(res.error());
      field.value[count] = elem.value;
    }
    return {};
  }
};

template <array_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using array_type = typename T::field_type;
    using array_element_field = create_field_from_array_of_records_v<T>;
    using read_impl_t = read_buffer_of_records<T, F, array_element_field>;

    constexpr auto array_len = extract_size_from_array_v<array_type>;
    auto reader = read_impl_t(field, field_list, ifs, array_len);
    auto res = reader();
    return res;
  }
};


template <vector_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using vector_element_field = create_field_from_vector_of_records_v<T>;
    using field_size = typename T::field_size;
    using read_impl_t = read_buffer_of_records<T, F, vector_element_field>;

    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    field.value.resize(len_to_read);
    auto reader = read_impl_t(field, field_list, ifs, len_to_read);
    auto res = reader();
    return res;
  }
};


// Forward declaration
template <field_list_like T>
constexpr auto struct_cast(std::ifstream&) -> std::expected<T, cast_error>;

template <struct_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs)
    : field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() const -> read_result {
    using field_list_t = extract_type_from_field_v<T>;
    auto res = struct_cast<field_list_t>(ifs);
    if(!res)
      return std::unexpected(res.error());
    // todo move?
    field.value = *res;
    return {};
  }
};


// todo restore constexpr
template <optional_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs): 
    field(field), field_list(field_list), ifs(ifs) {}
  
  constexpr auto operator()() -> read_result {
    if(!typename T::field_presence_checker{}(field_list)) {
      field.value = std::nullopt;
      return {};
    }
    using field_base_type_t = typename T::field_base_type;
    field_base_type_t base_field{};
    read_field<field_base_type_t, F> reader(base_field, field_list, ifs);
    auto res = reader();
    if(!res) 
      return std::unexpected(res.error());
    // todo is move guaranteed
    field.value = base_field.value;
    return {};
  }
};


// Helper function to read bytes into the variant
template<std::size_t idx, typename T, typename F, typename V>
struct read_variant_impl {
  V& variant;
  F& field_list;
  std::ifstream& ifs;
  std::size_t idx_r;

  constexpr explicit read_variant_impl(
    V& variant, 
    F& field_list,
    std::ifstream& ifs, 
    std::size_t idx_r) :
      variant(variant), field_list(field_list), ifs(ifs), idx_r(idx_r) {}

  constexpr auto operator()() -> read_result {
    if (idx_r != idx) 
      return {};

    T field;
    auto reader = read_field<T, F>(field, field_list, ifs);
    auto res = reader();
    if(!res)
      return std::unexpected(res.error());
    variant = std::move(field.value);
    return {};
  }
};


template <typename T, typename F, typename field_choices, typename idx_seq>
struct read_variant_helper;

template <typename T, typename F, typename... fields, std::size_t... idx>
struct read_variant_helper<T, F, field_choice_list<fields...>, std::index_sequence<idx...>> {
  T& field;
  F& field_list;
  std::ifstream& ifs;
  std::size_t idx_r;
  
  constexpr read_variant_helper(T& field, F& field_list, std::ifstream& ifs, std::size_t idx_r) 
    : field(field), field_list(field_list), ifs(ifs), idx_r(idx_r) {}
  
  constexpr auto operator()() -> read_result {
    read_result pipeline_seed{};
    return (
      pipeline_seed |
      ... | 
      read_variant_impl<idx, fields, F, typename T::field_type>(field.value, field_list, ifs, idx_r)
    );
  }
};


// todo restore constexpr
template <union_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  std::ifstream& ifs;

  constexpr read_field(T& field, F& field_list, std::ifstream& ifs): 
    field(field), field_list(field_list), ifs(ifs) {}

  constexpr auto operator()() -> read_result {
    using type_deduction_guide = typename T::type_deduction_guide;
    using field_choices = typename T::field_choices;
    constexpr auto max_type_index = T::variant_size;

    auto type_index_deducer = type_deduction_guide();
    auto type_index_result = type_index_deducer(field_list); 
    if(!type_index_result)
      return std::unexpected(type_index_result.error());

    auto idx_r = *type_index_result;
    using read_helper_t = 
      read_variant_helper<
        T, 
        F, 
        field_choices, 
        std::make_index_sequence<max_type_index>
      >;
    auto field_reader = read_helper_t(field, field_list, ifs, idx_r);
    auto field_read_res = field_reader();
    if(!field_read_res)
      return std::unexpected(field_read_res.error());
    return {};
  }
};

#endif // _FIELD_READER_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_reader.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/cast.hpp
#ifndef _CAST_HPP_
#define _CAST_HPP_

#include <print>

#include <expected>
 
// #include "field_meta.hpp"
 
 
// forward declaration
template <field_list_like T>
constexpr auto struct_cast(std::ifstream&) -> std::expected<T, cast_error>;

template <typename T>
struct struct_cast_impl;

template <typename... fields>
struct struct_cast_impl<struct_field_list<fields...>> {
  using S = struct_field_list<fields...>;
  using R = std::expected<S, cast_error>;

  constexpr auto operator()(std::ifstream& ifs) -> R {
    S field_list;
    read_result pipeline_seed{};
    auto res = (
      pipeline_seed |
      ... |
      read_field<fields, S>(static_cast<fields&>(field_list), field_list, ifs)
    );
    return res ? R(field_list) : std::unexpected(res.error());
  }
};

template <field_list_like T>
constexpr auto struct_cast(std::ifstream& ifs) -> std::expected<T, cast_error> {
  return struct_cast_impl<T>{}(ifs);
}

#endif // _CAST_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/cast.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/type_tag.hpp
#ifndef _TYPE_TAG_HPP_
#define _TYPE_TAG_HPP_
 
 
 
 
// todo is this required
// todo constraint T and size
template <trivial T, fixed_size_like S>
  requires (deduce_field_size<S>{}() <= sizeof(T))
struct trivial_tag {
  using type = T;
  using size = S;
};

template <field_list_like T>
struct struct_tag {
  using type = T;
  using size = field_size<size_dont_care>;
};

// todo how to handle array of array
template <trivial T, std::size_t N> 
struct fixed_buffer_tag {
  using type = std::array<T, N>;
  using size = field_size<fixed<N * sizeof(T)>>;
};

template <std::size_t N> 
struct fixed_string_tag {
  using type = fixed_string<N>;
  using size = field_size<fixed<N + 1>>;
};

template <trivial T, variable_size_like S> 
struct variable_buffer_tag {
  using type = std::vector<T>;
  using size = S;
};

template <variable_size_like S> 
struct variable_string_tag {
  using type = std::string;
  using size = S;
};

template <field_list_like T, std::size_t N>
struct array_of_record_tag {
  using type = std::array<T, N>;
  using size = field_size<size_dont_care>;
};

template <field_list_like T, variable_size_like S>
struct vector_of_record_tag {
  using type = std::vector<T>;
  using size = S;
};

template <typename T>
struct is_type_tag;

template <typename T, typename size>
struct is_type_tag<trivial_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename T, std::size_t size>
struct is_type_tag<fixed_buffer_tag<T, size>> {
  static constexpr bool res = true;
};

template <std::size_t size>
struct is_type_tag<fixed_string_tag<size>> {
  static constexpr bool res = true;
};

template <typename T, typename size>
struct is_type_tag<variable_buffer_tag<T, size>> {
  static constexpr bool res = true;
};

template <typename size>
struct is_type_tag<variable_string_tag<size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag<struct_tag<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_type_tag_v = is_type_tag<T>::res;

template <typename T>
concept type_tag_like = is_type_tag_v<T>;


#endif // _TYPE_TAG_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/type_tag.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/match_case.hpp
#ifndef _MATCH_CASE_HPP_
#define _MATCH_CASE_HPP_
 
// todo constrain to data types possible for fields
// todo constrain T?
template <auto v, type_tag_like T>
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

// End /home/hari/Code/struct_cast/worktree/memstream/include/match_case.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/clause.hpp
#ifndef _CLAUSE_HPP_
#define _CLAUSE_HPP_
 
 
// todo constrain v to function like object returning bool
template <typename eval, type_tag_like T>
  requires is_eval_bool_from_fields_v<eval>
struct clause {
  static constexpr auto e = eval{};
  using type_tag = T;
};


template <typename T>
struct is_clause;

template <typename T>
struct is_clause {
  static constexpr bool res = false;
};

template <typename eval, typename T>
struct is_clause<clause<eval, T>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_clause_v = is_clause<T>::res;

template <typename T>
concept clause_like = is_clause_v<T>;


#endif // _CLAUSE_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/clause.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/type_deduction_helper.hpp
#ifndef _TYPE_DEDUCTION_HELPER_HPP_
#define _TYPE_DEDUCTION_HELPER_HPP_
 
 
template <typename T>
concept type_condition_like = match_case_like<T> || clause_like<T>;

template <type_condition_like match_case>
struct type_from_type_condition;

template <type_condition_like match_case>
struct type_from_type_condition {
  using type = typename match_case::type_tag::type;
};

template <type_condition_like match_case>
struct size_from_type_condition;

template <type_condition_like match_case>
struct size_from_type_condition {
  using size = typename match_case::type_tag::size;
};

template <typename T>
using type_from_type_condition_v = type_from_type_condition<T>::type;

template <typename T>
using size_from_type_condition_v = size_from_type_condition<T>::size;

template <type_condition_like... cases>
struct variant_from_type_conditions {
  using variant = std::variant<type_from_type_condition_v<cases>...>;
};

template <type_condition_like... cases>
using variant_from_type_conditions_v = variant_from_type_conditions<cases...>::variant;

template <type_condition_like... cases>
struct size_choices_from_type_conditions {
  using choices = field_size<size_choices<size_from_type_condition_v<cases>...>>;
};

template <type_condition_like... cases>
using size_choices_from_type_conditions_v = size_choices_from_type_conditions<cases...>::choices;

#endif // _TYPE_DEDUCTION_HELPER_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/type_deduction_helper.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/type_ladder.hpp
#ifndef _TYPE_LADDER_HPP_
#define _TYPE_LADDER_HPP_
 
 
#include <expected>


// todo return type tag constructed from clause
template <typename... clauses>
struct type_ladder;

template <std::size_t idx, typename... clauses>
struct type_ladder_impl;

template <std::size_t idx>
struct type_ladder_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, cast_error> 
  {
    return std::unexpected(cast_error::type_deduction_failure);
  }
};

// todo constrain clause head and clause_rest
template <std::size_t idx, typename clause_head, typename... clause_rest>
struct type_ladder_impl<idx, clause_head, clause_rest...> {
  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, cast_error> 
  {
    bool eval_result = clause_head::e(field_list);
    if(eval_result) return idx;
    else return type_ladder_impl<idx + 1, clause_rest...>{}(field_list);
  }
};

template <typename clause_head, typename... clause_rest>
struct type_ladder<clause_head, clause_rest...> {
  // ? is this ok
  using variant = variant_from_type_conditions_v<clause_head, clause_rest...>;
  using sizes = size_choices_from_type_conditions_v<clause_head, clause_rest...>;

  template <typename... fields>
  constexpr auto operator()(const struct_field_list<fields...>& field_list) const -> 
    std::expected<std::size_t, cast_error> 
  {
    return type_ladder_impl<0, clause_head, clause_rest...>{}(field_list);
  }
};


#endif // _TYPE_LADDER_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/type_ladder.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/type_switch.hpp
#ifndef _TYPE_SWITCH_HPP_
#define _TYPE_SWITCH_HPP_

#include <expected>
 
 
 
template <std::size_t idx, typename... cases>
struct type_switch_impl;

template <std::size_t idx>
struct type_switch_impl<idx> {
  constexpr auto operator()(const auto&) const -> 
    std::expected<std::size_t, cast_error> 
  {
    return std::unexpected(cast_error::type_deduction_failure);
  }
};

// todo check if case and eval result match in terms of types
template <std::size_t idx, match_case_like match_case_head, match_case_like... match_case_rest>
struct type_switch_impl<idx, match_case_head, match_case_rest...> {
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, cast_error> 
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
  using variant = variant_from_type_conditions_v<case_head, case_rest...>;
  using sizes = size_choices_from_type_conditions_v<case_head, case_rest...>;

  template <typename... fields>
  constexpr auto operator()(const auto& v) const -> 
    std::expected<std::size_t, cast_error> 
  {
    return type_switch_impl<0, case_head, case_rest...>{}(v);
  } 
};

#endif // _TYPE_SWITCH_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/type_switch.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/type_deduction.hpp
#ifndef _TYPE_DEDUCTION_HPP_
#define _TYPE_DEDUCTION_HPP_

#include <expected>
 
 
 
 
 
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


template <fixed_string id>
using match_field = field_accessor<id>;

// todo constraints compute like
template <typename eval_expression, typename tswitch>
struct type<eval_expression, tswitch> {
  using expression = eval_expression;
  using type_switch = tswitch;
  using variant = tswitch::variant;
  using sizes = tswitch::sizes;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, cast_error> const {
    return type_switch{}(eval_expression{}(sfl)); 
  }
};

template <fixed_string id, typename tswitch>
struct type<match_field<id>, tswitch> {
  using type_switch = tswitch;
  using variant = tswitch::variant;
  using sizes = tswitch::sizes;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, cast_error> const {
    return type_switch{}(sfl[field_accessor<id>{}]); 
  }
};

// todo constraints
template <typename tladder>
struct type<tladder> {
  using type_ladder = tladder;
  using variant = tladder::variant;
  using sizes = tladder::sizes;

  template <typename... fields>
  auto operator()(const struct_field_list<fields...>& sfl)
    -> std::expected<std::size_t, cast_error> const {
    return type_ladder{}(sfl);
  }
};

// todo metafunction and concepts for constraining type

#endif // _TYPE_DEDUCTION_HPP_

// End /home/hari/Code/struct_cast/worktree/memstream/include/type_deduction.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/field_types.hpp
#ifndef _FIELD_TYPE_HPP_
#define _FIELD_TYPE_HPP_
 
 
 
 
 
 
struct always_true {
  constexpr auto operator()() -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}, with_fields<>>;

// todo better naming for this concept
template <typename size, typename field_type>
concept field_fits_to_underlying_type = deduce_field_size<size>{}() <= sizeof(field_type);

template <fixed_string id, integral T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
  requires field_fits_to_underlying_type<size_type, T>
using basic_field = field<id, T, size_type, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using fixed_array_field = field<id, std::array<T, N>, field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, field_list_like T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using array_of_records = field<id, std::array<T, N>, field_size<size_dont_care>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<fixed_string<N>>{}>
using fixed_string_field = field<id, fixed_string<N>, field_size<fixed<N + 1>>, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<T[N]>{}>
using c_arr_field = field<id, T[N], field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<char[N + 1]>{}>
using c_str_field = field<id, char[N + 1], field_size<fixed<N * sizeof(char) + 1>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, field_size<fixed<N>>, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, field_size<fixed<expected.size() + 1>>, eq{expected}>;

template <fixed_string id, integral T, fixed_size_like size, auto expected>
using magic_number = field<id, T, size, eq{expected}>;

// todo get vector length in bytes instead of size to read additional overload
// todo better naming and impl for from_field, since it is ambiguous about len or size in bytes
// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

// // todo check if this will work for all char types like wstring
template <fixed_string id, variable_size_like size, auto constraint_on_value = no_constraint<std::string>{}>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, field_size<size_dont_care>, no_constraint<T>{}>;


namespace static_test {
  using u32 = unsigned int;
  static inline auto is_eq_1 = [](auto a){ return a == 1; };
  // static_assert(is_optional_field_v<maybe_field<"a", u32, field_size<fixed<4>>, parse_if<is_eq_1, with_fields<"a">>>>);
  // static_assert(!is_optional_field_v<basic_field<"a", u32, field_size<fixed<4>>>>);
}
#endif /* _FIELD_TYPE_HPP_ */

// End /home/hari/Code/struct_cast/worktree/memstream/include/field_types.hpp

// Begin /home/hari/Code/struct_cast/worktree/memstream/include/struct_cast.hpp
#ifndef STRUCT_CAST_HPP
#define STRUCT_CAST_HPP
 
 
 
 
 
 
 
#endif // STRUCT_CAST_HPP

// End /home/hari/Code/struct_cast/worktree/memstream/include/struct_cast.hpp
