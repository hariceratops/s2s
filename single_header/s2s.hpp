#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <expected>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// Begin lib/containers/static_vector.hpp
#ifndef _STATIC_VECTOR_HPP_
#define _STATIC_VECTOR_HPP_
 
namespace s2s {
template <typename T, std::size_t N>
class static_vector {
public:
  constexpr static_vector() = default;
  template <typename... Args>
  constexpr static_vector(Args&&... entries) {
    (push_back(entries), ...);
  }
  constexpr auto push_back(const T& value) { 
    values[vec_size] = value; 
    vec_size++;
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { 
    return values[i]; 
  }
  [[nodiscard]] constexpr auto& operator[](std::size_t i) { 
    return values[i]; 
  }
  [[nodiscard]] constexpr auto begin() const { return &values[0]; }
  [[nodiscard]] constexpr auto end() const { return &values[0] + vec_size; }
  [[nodiscard]] constexpr auto size() const { return vec_size; }
  [[nodiscard]] constexpr auto empty() const { return not vec_size; }
  [[nodiscard]] constexpr auto capacity() const { return N; }

private:
  T values[N]{};
  std::size_t vec_size{0};
};
}

#endif /* _STATIC_VECTOR_HPP_ */

// End lib/containers/static_vector.hpp

// Begin lib/algorithms/algorithms.hpp
#ifndef _ALGORITHMS_HPP_
#define _ALGORITHMS_HPP_
 
// todo namespace algorithms
constexpr auto find_index(const std::ranges::range auto& ts, auto& t) -> std::size_t {
  for(auto i = 0u; i < ts.size(); ++i) {
    if(ts[i] == t) {
      return i;
    }
  }

  return ts.size();
}

constexpr auto find_index_if(const std::ranges::range auto& ts, auto predicate) -> std::size_t {
  for(auto i = 0u; i < ts.size(); ++i) {
    if(predicate(ts[i])) {
      return i;
    }
  }

  return ts.size();
}

constexpr auto equal_ranges(const std::ranges::range auto& xs, const std::ranges::range auto& ys) -> bool {
  if(xs.size() != ys.size()) return false;

  for(auto i = 0u; i < xs.size(); ++i) {
    if(xs[i] != ys[i])
      return false;
  }

  return true;
}

template <typename T>
constexpr void swap_objects(T& a, T& b) {
  T temp = a;
  a = b;
  b = temp;
}

constexpr auto sort_ranges(std::ranges::range auto& ts, auto predicate) {
  for(auto i = 0u; i < ts.size(); ++i) {
    for(auto j = 0u; j < ts.size() - 1; ++j) {
      if(predicate(ts[j + 1], ts[j])) {
        swap_objects(ts[j], ts[j + 1]);
      }
    }
  }
}


#endif /* _ALGORITHMS_HPP_ */

// End lib/algorithms/algorithms.hpp

// Begin lib/containers/static_map.hpp
#ifndef _STATIC_MAP_HPP_
#define _STATIC_MAP_HPP_
 
 
 
 
namespace s2s {
template <typename Key, typename Value>
class Node {
public:
  std::pair<Key, Value> element;

  constexpr Node() = default;
  constexpr Node(Key&& key, Value&& value): element(key, value) {}
  constexpr Node(const Key& key, const Value& value): element(key, value) {}
  constexpr Node(const Node& other): element(other.element) {}
  constexpr Node& operator=(const Node& other) {
    element.first = other.element.first;
    element.second = other.element.second;
    return *this;
  }

  constexpr bool operator<(const Node& rhs) const noexcept {
    return element.first < rhs.element.first;
  }

  constexpr const auto& operator*() const noexcept {
    return element;
  }

  constexpr const auto* operator->() const noexcept {
    return &element;
  }
};


template <typename Key, typename Value, std::size_t N /*, compare? */>
class static_map {
public:
  constexpr static_map() = default;
  constexpr static_map(const std::pair<Key, Value> (&entries)[N]): 
    map(generate_map<N>(entries, std::make_index_sequence<N>{})) {}
  constexpr auto operator[](const Key& key) const  -> std::optional<Value> {
    auto key_index = find_index_if(map, [&key](auto t){ return t.element.first == key; });
    if(key_index != map.size())
      return map[key_index].element.second;
    return std::nullopt;
  }
  [[nodiscard]] constexpr auto begin() const { return map.begin(); }
  [[nodiscard]] constexpr auto end() const { return map.end(); }
  [[nodiscard]] constexpr auto size() const { return map.size(); }
  [[nodiscard]] constexpr auto empty() const { return not map.size(); }
  [[nodiscard]] constexpr auto capacity() const { return N; }

private:
  static_vector<Node<Key, Value>, N> map{};
  template <std::size_t C, std::size_t... Is>
  constexpr auto generate_map(const std::pair<Key, Value> (&entries)[C], std::index_sequence<Is...>) {
    static_vector<Node<Key, Value>, N> m{};
    ([&]() {
      auto key = entries[Is].first;
      auto key_index = find_index_if(m, [&key](auto t){ return t.element.first == key; });
      if(key_index == m.size()) {
        m.push_back(Node(entries[Is].first, entries[Is].second));
        return;
      }
      m[key_index].element.second = entries[Is].second;
    }(), ...);
    sort_ranges(m, std::less<>{});
    return m;
  }
};
}

#endif /* _STATIC_MAP_HPP_ */

// End lib/containers/static_map.hpp

// Begin lib/containers/static_set.hpp
#ifndef _STATIC_SET_HPP_
#define _STATIC_SET_HPP_
 
 
namespace s2s {
template <typename T, std::size_t N>
class static_set {
public:
  constexpr static_set() = default;
  template <typename... Args>
  constexpr static_set(Args&&... entries) {
    (insert(entries), ...);
  }
  constexpr static_set(const static_vector<T, N>& vec) {
    for(auto value: vec) { insert(value); }
  }
  constexpr static_set(const std::array<T, N>& vec) {
    for(auto value: vec) { insert(value); }
  }
  constexpr auto insert(const T& value) { 
    if(find_index(*this, value) == set.size()) {
      set.push_back(value);
    }
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { 
    return set[i]; 
  }
  [[nodiscard]] constexpr auto& operator[](std::size_t i) { 
    return set[i]; 
  }
  [[nodiscard]] constexpr auto begin() const { return set.begin(); }
  [[nodiscard]] constexpr auto end() const { return set.end(); }
  [[nodiscard]] constexpr auto size() const { return set.size(); }
  [[nodiscard]] constexpr auto empty() const { return not set.size(); }
  [[nodiscard]] constexpr auto capacity() const { return N; }

private:
  static_vector<T, N> set{};
};
}

#endif /* _STATIC_SET_HPP_ */

// End lib/containers/static_set.hpp

// Begin lib/containers/static_optional.hpp
#ifndef _STATIC_OPTIONAL_HPP_
#define _STATIC_OPTIONAL_HPP_
 
namespace s2s {

struct static_nullopt_t {};
inline constexpr static_nullopt_t nullopt;

template <typename T>
struct static_optional {
  T value{};
  bool has_value{false};

  // Constructors
  constexpr static_optional() = default;
  constexpr static_optional(static_nullopt_t): has_value{false} {};
  constexpr static_optional(const static_nullopt_t&): has_value{false} {};
  constexpr static_optional(const T& val)
    : value(val), has_value(true) {}
  constexpr static_optional(const std::optional<T>& std_opt) {
    if(std_opt) {
      has_value = true;
      value = *std_opt;
    }
  }
  // constexpr static_optional(T&& val)
  //   : value(std::move(val)), has_value(true) {}

  [[nodiscard]] constexpr bool has() const noexcept { return has_value; }
  [[nodiscard]] constexpr const T& get() const { return value; }
  [[nodiscard]] constexpr operator bool() const noexcept { return has_value; }
  [[nodiscard]] constexpr const T& operator*() const { return value; }
  [[nodiscard]] constexpr T& operator*() { return value; }
  [[nodiscard]] constexpr const T* operator->() const { return &value; }
  [[nodiscard]] constexpr T* operator->() { return &value; }
  [[nodiscard]] constexpr auto operator<=>(const static_optional&) const = default;
  [[nodiscard]] constexpr bool operator==(static_nullopt_t&&) const { 
    if(has_value) 
      return true;
    return false; 
  };
};
}

#endif /* _STATIC_OPTIONAL_HPP_ */

// End lib/containers/static_optional.hpp

// Begin lib/containers/fixed_string.hpp
#ifndef _FIXED_STRING_HPP_
#define _FIXED_STRING_HPP_
 
 
 
 
namespace s2s {
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
  constexpr std::size_t size() const { return N; }
  constexpr auto to_sv() -> std::string_view const {
    return std::string_view{data()};
  }
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
} /* namespace s2s */

#endif // _FIXED_STRING_HPP_

// End lib/containers/fixed_string.hpp

// Begin field/field_accessor.hpp
#ifndef _FIELD_ACCESSOR_HPP_
#define _FIELD_ACCESSOR_HPP_
 
namespace s2s {
template <fixed_string id>
struct field_accessor {
  static constexpr auto field_id = id;
};
}

namespace s2s_literals {
template <s2s::fixed_string id>
constexpr auto operator""_f() {
  return s2s::field_accessor<id>{};
}
}

#endif // _FIELD_ACCESSOR_HPP_

// End field/field_accessor.hpp

// Begin lib/metaprog/fixed_string_list.hpp
#ifndef _FIXED_STRING_LIST_HPP_
#define _FIXED_STRING_LIST_HPP_


// status: might be deprecated due to value MP
 
namespace s2s {
template <fixed_string... fs>
struct fixed_string_list {};

template <typename T>
struct is_field_name_list;

template <typename T>
struct is_field_name_list {
  static constexpr bool res = false;
};

template <fixed_string... fs>
struct is_field_name_list<fixed_string_list<fs...>> {
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

template <fixed_string... fs>
inline constexpr auto with_fields = fixed_string_list<fs...>{};

// A name list is spelled either as bare ids or as one with_fields value. The
// normalization sits here, at the user-facing aliases, and never at `compute`
// itself — every internal pattern match on compute_t<f, R, fixed_string_list<...>>
// therefore stays untouched.
template <auto... ids>
struct names_of {
  using type = fixed_string_list<ids...>;
};

template <auto list>
  requires field_name_list<std::remove_cvref_t<decltype(list)>>
struct names_of<list> {
  using type = std::remove_cvref_t<decltype(list)>;
};

template <auto... ids>
using field_names_of = typename names_of<ids...>::type;
} /* namespace s2s */


#endif // _FIXED_STRING_LIST_HPP_

// End lib/metaprog/fixed_string_list.hpp

// Begin field_size/field_size.hpp
#ifndef _FIELD_SIZE_HPP_
#define _FIELD_SIZE_HPP_
 
 
namespace s2s {
// A size is a value of an empty structural type, and every classification
// trait keys on that value's *type*. The reason is not style: a class-type
// NTTP is a non-deduced context in a partial specialization, so
// `field<id, T, field_accessor<len_source>{}, c>` cannot deduce `len_source`.
// Anything a trait needs to match on therefore has to live in the type.
//
// byte_count is the one exception — the width lives in the value, because no
// trait matches on a width. Only deduce_field_size reads it, and it reads the
// value. That exception is what buys a plain non-template operator""_B.
struct byte_count {
  std::size_t count{};
};

struct size_dont_care_t {};

// Safety is on by default: a field that declares nothing still gets a ceiling.
// The macro is the one global knob — raise it, or set it to SIZE_MAX to turn
// the defaults off wholesale. It cannot reach a declared max_bytes, which is
// the schema author's intent rather than the library's guess.
//
// 16 MiB: far above what fields in real binary formats carry, far below what a
// corrupt u32 can claim, and survivable as an accidental allocation.
#ifndef S2S_DEFAULT_MAX_BYTES
#define S2S_DEFAULT_MAX_BYTES (16u * 1024u * 1024u)
#endif

inline constexpr std::size_t default_max_bytes = S2S_DEFAULT_MAX_BYTES;

template <std::size_t N>
struct max_byte_count_t {
  static constexpr std::size_t count = N;
};

// "no bound declared" is its own type rather than max_byte_count_t<SIZE_MAX>,
// so the default stays a deferral to whatever the macro says at include time
// rather than a number baked into every field.
struct use_default_bound_t {};

template <auto callable, field_name_list req_fields>
struct size_from_fields_t {
  static constexpr auto f = callable;
  static constexpr auto req_field_list = req_fields{};
};

// todo size type for holding multiple sizes in case of union fields
template <auto... sizes>
struct size_choices_t {
  static constexpr auto num_of_choices = sizeof...(sizes);
};

// The spellings a schema writes.
template <fixed_string id>
inline constexpr auto len_from_field = field_accessor<id>{};

// todo constraint for callable
template <auto callable, fixed_string... ids>
inline constexpr auto size_from_fields = size_from_fields_t<callable, field_names_of<ids...>>{};

template <auto callable, fixed_string... ids>
inline constexpr auto len_from_fields = size_from_fields<callable, ids...>;

inline constexpr auto size_dont_care = size_dont_care_t{};

template <std::size_t N>
inline constexpr auto max_bytes = max_byte_count_t<N>{};

inline constexpr auto use_default_bound = use_default_bound_t{};

template <auto... sizes>
inline constexpr auto size_choices = size_choices_t<sizes...>{};

// Identity on an already-value size: 4_B is byte_count{4}.
template <auto size>
inline constexpr auto field_size = size;

// decltype of an auto NTTP parameter is unqualified, but decltype of the
// variable templates above is const-qualified. Strip in one place.
template <auto size>
using size_type_of = std::remove_cvref_t<decltype(size)>;

// Recovering the parameters a trait matched on, out of the size value's type.
template <typename S>
struct len_source_of;

template <fixed_string id>
struct len_source_of<field_accessor<id>> {
  static constexpr auto value = id;
};

template <typename T>
struct is_bound_option {
  static constexpr bool res = false;
};

template <std::size_t N>
struct is_bound_option<max_byte_count_t<N>> {
  static constexpr bool res = true;
};

template <>
struct is_bound_option<use_default_bound_t> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_bound_option_v = is_bound_option<T>::res;

template <typename T>
concept bound_option_like = is_bound_option_v<T>;

// Resolving a declared bound, or deferring to the macro.
template <typename B>
struct bound_value {
  static constexpr std::size_t value = default_max_bytes;
};

template <std::size_t N>
struct bound_value<max_byte_count_t<N>> {
  static constexpr std::size_t value = N;
};

template <auto bound>
inline constexpr std::size_t bound_in_bytes = bound_value<size_type_of<bound>>::value;

// Metafunctions for checking if a type is a size type
template <typename T>
struct is_fixed_size {
  static constexpr bool res = false;
};

template <>
struct is_fixed_size<byte_count> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_fixed_size_v = is_fixed_size<T>::res;

template <typename T>
struct is_variable_size {
  static constexpr bool res = false;
};

template <fixed_string id>
struct is_variable_size<field_accessor<id>> {
  static constexpr bool res = true;
};

template <auto callable, field_name_list ids>
struct is_variable_size<size_from_fields_t<callable, ids>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_variable_size_v = is_variable_size<T>::res;

// A size produced by a user callable rather than read from a single field.
// The distinction matters only on the write path: len_from_field can be
// inverted and derived, this cannot, so it can only be verified.
template <typename T>
struct is_computed_size {
  static constexpr bool res = false;
};

template <auto callable, field_name_list ids>
struct is_computed_size<size_from_fields_t<callable, ids>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_computed_size_v = is_computed_size<T>::res;

// Concepts for checking if a type is a size type
template <typename T>
concept fixed_size_like = is_fixed_size_v<T>;

template <typename T>
concept variable_size_like = is_variable_size_v<T>;

template <typename T>
concept atomic_size = fixed_size_like<T> || variable_size_like<T>;

template <typename T>
struct is_selectable_size {
  static constexpr bool res = false;
};

template <auto... sizes>
  requires (atomic_size<size_type_of<sizes>> && ...)
struct is_selectable_size<size_choices_t<sizes...>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_selectable_size_v = is_selectable_size<T>::res;

template <typename T>
concept selectable_size_like = is_selectable_size_v<T>;

template <typename T>
concept is_size_like = fixed_size_like<T>    ||
                       variable_size_like<T> ||
                       selectable_size_like<T>;

template <typename T>
struct is_size_dont_care {
  static constexpr bool res = false;
};

template <>
struct is_size_dont_care<size_dont_care_t> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_size_dont_care_v = is_size_dont_care<T>::res;

template <typename T>
concept size_dont_care_like = is_size_dont_care_v<T>;

} /* namespace s2s */

namespace s2s_literals {
constexpr auto operator""_B(unsigned long long n) -> s2s::byte_count {
  return s2s::byte_count{static_cast<std::size_t>(n)};
}
}

#endif // _FIELD_SIZE_HPP_

// End field_size/field_size.hpp

// Begin lib/metaprog/typelist.hpp
#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_


// status: might be deprecated due to value MP
 
 
namespace s2s {
namespace typelist {
struct null {};

template <typename... ts>
struct list;

template <typename... ts>
struct list{};

template <>
struct list<>{};

template <typename... ts>
struct any_of;

template <typename... ts>
struct any_of {};

template <typename t>
struct any_of<list<>, t> { static constexpr bool res = false; };

template <typename t, typename... rest>
struct any_of<list<t, rest...>, t> { static constexpr bool res = true; };

template <typename t, typename u, typename... rest>
struct any_of<list<u, rest...>, t> { static constexpr bool res = false || any_of<list<rest...>, t>::res; };

template <typename list, typename type>
inline constexpr bool any_of_v = any_of<list, type>::res;

template <typename... ts>
struct all_are_same;

template <>
struct all_are_same<list<>> {
  static constexpr auto all_same = true;
};

template <typename T>
struct all_are_same<list<T>> {
  static constexpr auto all_same = true;
};

template <typename T, typename U, typename... rest>
struct all_are_same<list<T, U, rest...>> {
  static constexpr auto all_same = std::is_same_v<T, U> && all_are_same<list<U, rest...>>::all_same;
};

template <typename T, typename... rest>
struct all_are_same<list<T, rest...>> {
  static constexpr auto all_same = false;
};

template <typename L>
inline constexpr bool all_are_same_v = all_are_same<L>::all_same;

template <typename... ts>
struct front;

template <typename t, typename... ts>
struct front<list<t, ts...>> {
  using front_t = t;
};

template <>
struct front<list<>> {
  using front_t = null;
};

template <typename L>
using front_t = typename front<L>::front_t;

} // namespace list
} /* namespace s2s */

#endif // _TYPELIST_HPP_

// End lib/metaprog/typelist.hpp

// Begin field_list/field_list_base.hpp
#ifndef _FIELD_LIST_BASE_HPP_
#define _FIELD_LIST_BASE_HPP_
 
namespace s2s {
struct struct_field_list_base {};

template <typename T>
concept field_list_like = std::is_base_of_v<struct_field_list_base, T>;
} /* namespace s2s */

#endif // _FIELD_LIST_BASE_HPP_

// End field_list/field_list_base.hpp

// Begin lib/s2s_traits/type_traits.hpp
#ifndef _S2S_TYPE_TRAITS_HPP_
#define _S2S_TYPE_TRAITS_HPP_
 
 
 
 
 
namespace s2s {
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
template <typename T>
struct is_optional_like;

template <typename T>
struct is_optional_like {
  static inline constexpr bool res = false;
};

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

// todo vector of vectors or vector of arrays?
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
  static constexpr auto value = N;
};

template <typename T>
inline constexpr std::size_t extract_size_from_array_v = extract_size_from_array<T>::value;

template <typename T>
concept variable_sized_buffer_like = vector_like<T> || string_like<T>;

template <typename T>
concept constant_sized_like = fixed_buffer_like<T> || trivial<T>;

template <typename T>
concept buffer_like = fixed_buffer_like<T> || variable_sized_buffer_like<T>;
} /* namespace s2s */

#endif // _S2S_TYPE_TRAITS_HPP_

// End lib/s2s_traits/type_traits.hpp

// Begin field_validation/field_value_constraints.hpp
#ifndef _FIELD_VALUE_CONSTRAINTS_HPP_
#define _FIELD_VALUE_CONSTRAINTS_HPP_ 
 
 
 
 
 
 
namespace s2s {
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
  requires (typelist::all_are_same_v<typelist::list<T, Ts...>>)
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
  requires (typelist::all_are_same_v<typelist::list<ts...>>)
struct is_in_open_range {
  std::array<range<t>, 1 + sizeof...(ts)> open_ranges;

  constexpr is_in_open_range(range<t> r, range<ts>... rs) : open_ranges{r, rs...} {
    std::sort(open_ranges.begin(), open_ranges.end(), [](const range<t>& r1, const range<t>& r2) {
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
}

#endif // _FIELD_VALUE_CONSTRAINTS_HPP_

// End field_validation/field_value_constraints.hpp

// Begin field/field.hpp
#ifndef _FIELD_HPP_
#define _FIELD_HPP_
 
 
namespace s2s {
template <fixed_string id,
          typename T,
          auto size,
          auto constraint_on_value,
          auto max_byte_bound = use_default_bound>
struct field {
  using field_type = T;
  static constexpr auto field_size = size;
  // The ceiling the read path applies before allocating this field. Defaulted,
  // so every construction site that predates bounds keeps compiling.
  static constexpr auto field_bound = max_byte_bound;

  static constexpr auto field_id = id;
  static constexpr auto constraint_checker = constraint_on_value;
  field_type value{};
};

template <typename T>
struct to_optional_field;

template <fixed_string id, typename T, auto size, auto constraint_on_value, auto bound>
struct to_optional_field<field<id, T, size, constraint_on_value, bound>> {
  using res = field<id, std::optional<T>, size, no_constraint<std::optional<T>>{}, bound>;
};

template <typename T>
using to_optional_field_v = to_optional_field<T>::res;

template <typename T>
struct no_variance_field;

template <fixed_string id,
          typename T,
          auto size,
          auto bound>
struct no_variance_field<field<id, T, size, no_constraint<T>{}, bound>> {
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




template <typename base_field,
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

template <fixed_string id, typename T, auto size>
struct to_field_choice {
  using field_choice = field<id, T, size, no_constraint<T>{}>;
};

template <fixed_string id, typename T, auto size>
using to_field_choice_v = to_field_choice<id, T, size>::field_choice;

template <fixed_string id, typename... types, auto... sizes>
struct to_field_choices<id, std::variant<types...>, size_choices_t<sizes...>> {
  using choices = field_choice_list<to_field_choice_v<id, types, sizes>...>;
};

template <fixed_string id, typename type_deducer>
struct union_field: public 
    field<
      id, 
      typename type_deducer::variant, 
      size_dont_care, 
      no_constraint<typename type_deducer::variant>{}
    > 
{
  using type_deduction_guide = type_deducer;
  static constexpr auto variant_size = std::variant_size_v<typename type_deducer::variant>;
  using field_choices = typename to_field_choices<
      id, 
      typename type_deducer::variant, 
      typename type_deducer::sizes
    >::choices;
};

} /* namespace s2s */

#endif // _FIELD_HPP_

// End field/field.hpp

// Begin lib/metaprog/mp.hpp
#ifndef _MP_HPP_
#define _MP_HPP_
 
 
 
namespace meta {
using type_identifier = std::size_t;

namespace meta_impl {
// todo: fix warning due to friend injection
template <type_identifier>
struct type_id_key {
  constexpr auto friend get(type_id_key);
};

template <typename T>
struct type_id_value {
  using value_type = T;

  template <std::size_t left = 0u, std::size_t right = 1024u - 1u>
  static constexpr auto gen() -> std::size_t {
    if constexpr (left >= right) {
      constexpr bool exists = requires { get(type_id_key<type_identifier{left}>{}); };
      return (exists ? left + 1 : left);
    } else if constexpr (
      constexpr std::size_t mid = left + (right - left) / 2u; 
      requires { get(type_id_key<type_identifier{mid}>{}); }) 
    {
      return gen<mid + 1u, right>();
    } else {
      return gen<left, mid - 1u>();
    }
  }

  static constexpr auto id = type_identifier{gen()};

  constexpr auto friend get(type_id_key<id>) { 
    return type_id_value{}; 
  }
};
}

template<class T>
inline constexpr type_identifier type_id = meta_impl::type_id_value<T>::id;

template<type_identifier our_id>
using type_of = typename decltype(get(meta_impl::type_id_key<our_id>{}))::value_type;

template<class Fn, class T = decltype([]{})>
[[nodiscard]] inline constexpr auto invoke(Fn&& fn, type_identifier meta) {
  constexpr auto dispatch = [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
    return std::array{
      []<type_identifier N> {
        return +[](Fn fn) {
          if constexpr (requires { fn.template operator()<N>(); }) {
            return fn.template operator()<N>();
          }
        };
      }.template operator()<type_identifier{Ns}>()...
    };
  }(std::make_index_sequence<std::size_t{type_id<T>}>{});
  return dispatch[std::size_t{meta}](fn);
}

template <template<typename...> typename T, class... Ts, auto = []{}>
[[nodiscard]] inline constexpr auto invoke(type_identifier id) {
  return invoke([]<type_identifier id> {
    using type = type_of<id>;
    if constexpr(requires { T<Ts..., type>::value; }) {
      return T<Ts..., type>::value;
    } else {
      return type_id<typename T<Ts..., type>::type>;
    }
  }, id);
}
}


#endif /* _MP_HPP_ */

// End lib/metaprog/mp.hpp

// Begin field/field_type_info.hpp
#ifndef _FIELD_NODE_HPP_
#define _FIELD_NODE_HPP_
 
// todo better name
struct field_type_info {
  meta::type_identifier id;
  std::size_t occurs_at_idx;
};

#endif /* _FIELD_NODE_HPP_ */

// End field/field_type_info.hpp

// Begin type_deduction/type/type.hpp
#ifndef _TYPE_HPP_
#define _TYPE_HPP_
 
namespace s2s {
template <typename... Args>
struct type;


template <fixed_string id>
using match_field = field_accessor<id>;

// todo constraints compute like
template <typename eval_expression, typename _switch>
struct type<eval_expression, _switch> {
  using expression = eval_expression;
  using type_switch = _switch;
  using variant = _switch::variant;
  using sizes = _switch::sizes;
};

template <fixed_string id, typename _switch>
struct type<match_field<id>, _switch> {
  using type_switch = _switch;
  using variant = _switch::variant;
  using sizes = _switch::sizes;
};

// todo constraints
template <typename ladder>
struct type<ladder> {
  using type_ladder = ladder;
  using variant = ladder::variant;
  using sizes = ladder::sizes;
};
} /* namespace s2s */


#endif // _TYPE_HPP_

// End type_deduction/type/type.hpp

// Begin error/cast_error.hpp
#ifndef _CAST_ERROR_HPP_
#define _CAST_ERROR_HPP_
 
 
namespace s2s {
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure,
  // Two parts of the struct imply different lengths for the same data — a
  // cross-field disagreement, not a value that is wrong on its own terms.
  // Appended rather than inserted so the existing enumerators keep their
  // values.
  found_contradicting_length,
  // A length off the wire that cannot be allocated: it would overflow the byte
  // count, or exceed the field's ceiling. Distinct from buffer_exhaustion,
  // which means the stream ran dry *during* a read — this one fires before any
  // allocation happens, which is the whole point of it.
  excessive_length
};


struct cast_error {
  error_reason failure_reason;
  std::string_view failed_at;
};


using rw_result = std::expected<void, error_reason>;
using cast_result = std::expected<void, cast_error>;


// Both directions fold their per-field steps through these, so they live here
// rather than in the read path's headers — the amalgamated header is a single
// translation unit, where a second definition would be an error.
constexpr auto operator|(const cast_result& res, auto&& callable) -> cast_result
{
  return res ? callable() : std::unexpected(res.error());
}

constexpr auto operator|(const rw_result& res, auto&& callable) -> rw_result
{
  return res ? callable() : std::unexpected(res.error());
}

} /* namespace s2s */

#endif // _CAST_ERROR_HPP_

// End error/cast_error.hpp

// Begin field_size/comptime_field_size_deduce.hpp
#ifndef _COMPTIME_FIELD_SIZE_DEDUCE_HPP_
#define _COMPTIME_FIELD_SIZE_DEDUCE_HPP_
 
namespace s2s {
// The one value-keyed template on the size axis: byte_count carries its width
// in the value, so this reads the value rather than the type. The variable
// size case lives in field_size_deduce.hpp, which this header must not pull
// in — it would drag field_list.hpp along with it.
template <auto size>
struct deduce_field_size;

template <auto size>
  requires fixed_size_like<size_type_of<size>>
struct deduce_field_size<size> {
  constexpr auto operator()() const -> std::size_t {
    return size.count;
  }
};
} /* namespace s2s */


#endif // _COMPTIME_FIELD_SIZE_DEDUCE_HPP_

// End field_size/comptime_field_size_deduce.hpp

// Begin type_deduction/utils/type_tags.hpp
#ifndef _TYPE_TAGS_
#define _TYPE_TAGS_
 
 
 
 
namespace s2s {
template <trivial T, auto S>
  requires fixed_size_like<size_type_of<S>> && (deduce_field_size<S>{}() <= sizeof(T))
struct as_trivial {
  using type = T;
  static constexpr auto size = S;
};

template <field_list_like T>
struct as_struct {
  using type = T;
  static constexpr auto size = size_dont_care;
};

// todo how to handle array of array
template <trivial T, std::size_t N> 
struct as_fixed_arr {
  using type = std::array<T, N>;
  static constexpr auto size = byte_count{N * sizeof(T)};
};

template <std::size_t N> 
struct as_fixed_string {
  using type = fixed_string<N>;
  static constexpr auto size = byte_count{N + 1};
};

template <trivial T, auto S> 
  requires variable_size_like<size_type_of<S>>
struct as_vec {
  using type = std::vector<T>;
  static constexpr auto size = S;
};

template <auto S> 
  requires variable_size_like<size_type_of<S>>
struct as_string {
  using type = std::string;
  static constexpr auto size = S;
};

template <field_list_like T, std::size_t N>
struct as_arr_of_records {
  using type = std::array<T, N>;
  static constexpr auto size = size_dont_care;
};

template <field_list_like T, auto S>
  requires variable_size_like<size_type_of<S>>
struct as_vec_of_records {
  using type = std::vector<T>;
  static constexpr auto size = S;
};

template <typename T>
struct is_type_tag;

template <typename T, auto size>
struct is_type_tag<as_trivial<T, size>> {
  static constexpr bool res = true;
};

template <typename T, std::size_t size>
struct is_type_tag<as_fixed_arr<T, size>> {
  static constexpr bool res = true;
};

template <std::size_t size>
struct is_type_tag<as_fixed_string<size>> {
  static constexpr bool res = true;
};

template <typename T, auto size>
struct is_type_tag<as_vec<T, size>> {
  static constexpr bool res = true;
};

template <auto size>
struct is_type_tag<as_string<size>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_type_tag<as_struct<T>> {
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
} /* namespace s2s */

#endif // _TYPE_TAGS_

// End type_deduction/utils/type_tags.hpp

// Begin type_deduction/switch/match_case.hpp
#ifndef _MATCH_CASE_HPP_
#define _MATCH_CASE_HPP_
 
namespace s2s {
// todo constrain to data types possible for fields
template <auto v, type_tag_like T>
struct match_case {
  static constexpr auto value = v;
  using type_tag = T;
};
} /* namespace s2s */

#endif // _MATCH_CASE_HPP_

// End type_deduction/switch/match_case.hpp

// Begin type_deduction/switch/match_case_traits.hpp
#ifndef _MATCH_CASE_TRAITS_HPP_
#define _MATCH_CASE_TRAITS_HPP_
 
namespace s2s {
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
} /* namespace s2s */

#endif // _MATCH_CASE_TRAITS_HPP_

// End type_deduction/switch/match_case_traits.hpp

// Begin field_compute/computation_from_fields.hpp
#ifndef _COMPUTATION_FROM_FIELDS_HPP_
#define _COMPUTATION_FROM_FIELDS_HPP_
 
namespace s2s {
// compute_t keeps a field_name_list *type* so that every internal pattern
// match on it stays a plain partial specialization; `compute` is the spelling a
// schema writes, and normalizes bare ids into that type.
template <auto callable, typename R, field_name_list Fs>
struct compute_t;

template <auto callable, typename R, fixed_string... req_fields>
struct compute_t<callable, R, fixed_string_list<req_fields...>>{
};

template <auto callable, typename R, fixed_string... ids>
using compute = compute_t<callable, R, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using eval_bool_from_fields = compute_t<callable, bool, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using predicate = compute_t<callable, bool, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using eval_size_from_fields = compute_t<callable, std::size_t, field_names_of<ids...>>;

template <auto callable, fixed_string... ids>
using parse_if = eval_bool_from_fields<callable, ids...>;
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_HPP_

// End field_compute/computation_from_fields.hpp

// Begin field_compute/computation_from_fields_traits.hpp
#ifndef _COMPUTATION_FROM_FIELDS_TRAITS_HPP_
#define _COMPUTATION_FROM_FIELDS_TRAITS_HPP_
 
 
 
using namespace s2s_literals;

namespace s2s {
template <typename T>
struct is_compute_like;

template <auto callable, typename R, fixed_string... req_fields>
struct is_compute_like<compute_t<callable, R, fixed_string_list<req_fields...>>> {
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
struct is_eval_bool_from_fields<compute_t<callable, bool, fixed_string_list<req_fields...>>> {
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
struct is_eval_size_from_fields<compute_t<callable, std::size_t, fixed_string_list<req_fields...>>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_eval_size_from_fields {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_eval_size_from_fields_v = is_eval_size_from_fields<T>::res;
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_TRAITS_HPP_

// End field_compute/computation_from_fields_traits.hpp

// Begin type_deduction/if_else_ladder/clause.hpp
#ifndef _CLAUSE_HPP_
#define _CLAUSE_HPP_
 
 
namespace s2s {

template <typename T>
concept evaluates_to_bool = is_eval_bool_from_fields_v<T>;

template <evaluates_to_bool eval, type_tag_like T>
struct branch {
  using expression = eval;
  using type_tag = T;
};
} /* namespace s2s */

#endif // _CLAUSE_HPP_

// End type_deduction/if_else_ladder/clause.hpp

// Begin type_deduction/if_else_ladder/clause_traits.hpp
#ifndef _CLAUSE_TRAITS_HPP_
#define _CLAUSE_TRAITS_HPP_
 
namespace s2s {
template <typename T>
struct is_branch;

template <typename T>
struct is_branch {
  static constexpr bool res = false;
};

template <typename eval, typename T>
struct is_branch<branch<eval, T>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_branch_v = is_branch<T>::res;

template <typename T>
concept branch_like = is_branch_v<T>;
} /* namespace s2s */

#endif // _CLAUSE_TRAITS_HPP_

// End type_deduction/if_else_ladder/clause_traits.hpp

// Begin type_deduction/utils/helper.hpp
#ifndef _HELPER_HPP_
#define _HELPER_HPP_
 
 
namespace s2s {
using type_deduction_idx = std::optional<std::size_t>;
using type_deduction_res = std::expected<std::size_t, error_reason>;

constexpr auto operator|(const type_deduction_idx& res, auto&& callable) -> type_deduction_idx {
  return res ? res : callable();
}

template <typename T>
concept type_condition_like = match_case_like<T> || branch_like<T>;

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
  static constexpr auto size = match_case::type_tag::size;
};

template <typename T>
using type_from_type_condition_v = type_from_type_condition<T>::type;

template <typename T>
inline constexpr auto size_from_type_condition_v = size_from_type_condition<T>::size;

template <type_condition_like... cases>
struct variant_from_type_conditions {
  using variant = std::variant<type_from_type_condition_v<cases>...>;
};

template <type_condition_like... cases>
using variant_from_type_conditions_v = variant_from_type_conditions<cases...>::variant;

template <type_condition_like... cases>
struct size_choices_from_type_conditions {
  using choices = size_choices_t<size_from_type_condition_v<cases>...>;
};

template <type_condition_like... cases>
using size_choices_from_type_conditions_v = size_choices_from_type_conditions<cases...>::choices;
} /* namespace s2s */ 

#endif // _TYPE_DEDUCTION_HELPER_HPP_

// End type_deduction/utils/helper.hpp

// Begin type_deduction/if_else_ladder/ladder.hpp
#ifndef _LADDER_HPP_
#define _LADDER_HPP_
 
 
namespace s2s {
template <branch_like... branches>
  requires (sizeof...(branches) > 0)
struct type_if_else {
  // todo possibly unused
  using variant = variant_from_type_conditions_v<branches...>;
  using sizes = size_choices_from_type_conditions_v<branches...>;
};
} /* namespace s2s */

#endif // _LADDER_HPP_

// End type_deduction/if_else_ladder/ladder.hpp

// Begin field_list/field_list_metadata.hpp
#ifndef _FIELD_LIST_METADATA_HPP_
#define _FIELD_LIST_METADATA_HPP_
 
 
 
 
 
 
 
 
 
 
 
namespace s2s {
// todo fix these numbers and possibly generate them
static inline constexpr std::size_t max_dep_count_per_field = 8;
static inline constexpr std::size_t max_union_choices = 8;
static inline constexpr std::size_t max_dep_count_per_struct = max_dep_count_per_field * max_union_choices;
static inline constexpr std::size_t max_field_count = 256;


using sv = std::string_view;
using dep_vec = static_vector<sv, max_dep_count_per_struct>;
using field_table_t = static_map<sv, field_type_info, max_field_count>;
using dependency_table_t = static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>;

// A computed size keeps its field-name list in the size value's type; deducing
// the pack out of the argument recovers it without a metafunction.
template <fixed_string... fs>
constexpr auto deps_of(fixed_string_list<fs...>) -> static_vector<sv, max_dep_count_per_struct> {
  return static_vector<sv, max_dep_count_per_struct>(as_sv(fs)...);
}

// extract dependencies metafunction
template <typename T>
struct extract_length_dependencies;

template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires fixed_size_like<size_type_of<size>>
struct extract_length_dependencies<
  field<id, T, size, constraint, bound>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires size_dont_care_like<size_type_of<size>>
struct extract_length_dependencies<
  field<id, T, size, constraint, bound>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct extract_length_dependencies<
  field<id, T, size, constraint, bound>
>
{
  static constexpr auto value =
    static_vector<sv, max_dep_count_per_struct>(as_sv(len_source_of<size_type_of<size>>::value));
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires is_computed_size_v<size_type_of<size>>
struct extract_length_dependencies<
  field<id, T, size, constraint, bound>
>
{
  static constexpr auto value = deps_of(size_type_of<size>::req_field_list);
};

template <fixed_string id, typename T, auto size, auto constraint, 
          typename present_only_if, typename optional>
struct extract_length_dependencies<
  maybe_field<field<id, T, size, constraint>, present_only_if, optional>
>
{
  using f = field<id, T, size, constraint>;
  static constexpr auto value = extract_length_dependencies<f>::value;
};

template <std::size_t N>
constexpr auto flatten(const dep_vec (&vecs)[N]) -> dep_vec {
  dep_vec vec;
  for(auto i = 0u; i < N; i++) {
    for(auto& elem: vecs[i]) {
      vec.push_back(elem);
    }
  }
  return vec;
}

template <typename... Ts>
struct extract_length_dependencies_from_field_choices;

template <typename... Ts>
struct extract_length_dependencies_from_field_choices<field_choice_list<Ts...>>{
  static constexpr dep_vec deps[64] = {dep_vec(extract_length_dependencies<Ts>::value)...};
  static constexpr auto value = flatten(deps);
};

template <typename... Ts>
inline constexpr auto extract_length_dependencies_from_field_choices_v = extract_length_dependencies_from_field_choices<Ts...>::value;

template <fixed_string id, typename type_deducer>
struct extract_length_dependencies<
  union_field<id, type_deducer>
> 
{
  using field = union_field<id, type_deducer>;
  using field_choices = typename field::field_choices;
  static constexpr auto value = extract_length_dependencies_from_field_choices_v<field_choices>;
};
 

template <typename T>
inline constexpr auto extract_length_dependencies_v = extract_length_dependencies<T>::value;


template <auto callable, typename R, field_name_list Fs>
struct compute_t;

template <typename T>
struct extract_parse_dependencies;

template <typename T>
struct extract_parse_dependencies {
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound,
          auto callable, fixed_string... req_fields, typename optional>
struct extract_parse_dependencies<
  maybe_field<field<id, T, size, constraint, bound>, compute_t<callable, bool, fixed_string_list<req_fields...>>, optional>
>
{
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>(as_sv(req_fields)...);
};

template <typename T>
inline constexpr auto extract_parse_dependencies_v = extract_parse_dependencies<T>::value;


template <typename T>
struct extract_type_deduction_dependencies;

template <typename T>
struct extract_type_deduction_dependencies {
  static constexpr auto value = static_vector<sv, max_dep_count_per_struct>();
};

template <fixed_string id, fixed_string matched_id, typename type_switch>
struct extract_type_deduction_dependencies<
  union_field<
    id,
    type<match_field<matched_id>, type_switch>
  >
> 
{
  static constexpr auto value = dep_vec(as_sv(matched_id));
};

template <fixed_string id, auto callable, typename R, fixed_string... req_fields, typename type_switch>
struct extract_type_deduction_dependencies<
  union_field<
    id,
    type<compute_t<callable, R, fixed_string_list<req_fields...>>, type_switch>
  >
> 
{
  static constexpr auto value = dep_vec(as_sv(req_fields)...);
};

template <typename T>
struct extract_req_fields_from_clause;

template <auto callable, fixed_string... req_fields, type_tag_like T>
struct extract_req_fields_from_clause<
  branch<
    compute_t<callable, bool, fixed_string_list<req_fields...>>,
    T
  >
>
{
  static constexpr auto value = dep_vec(as_sv(req_fields)...);
};

template <typename T>
inline constexpr auto extract_req_fields_from_clause_v = extract_req_fields_from_clause<T>::value;

// todo better implementation
constexpr auto remove_duplicates(const dep_vec& vec) -> dep_vec {
  static_set<sv, max_dep_count_per_struct> set(vec);
  dep_vec res;
  for(auto item: set)
    res.push_back(item);
  return res;
}

// template<typename...>... typename clauses?
template <fixed_string id, typename... clauses>
struct extract_type_deduction_dependencies<
  union_field<
    id,
    type<type_if_else<clauses...>>
  >
> 
{
  static constexpr dep_vec deps[64] = {dep_vec(extract_req_fields_from_clause_v<clauses>)...};
  static constexpr auto flat_values = flatten(deps);
  static constexpr auto value = remove_duplicates(flat_values);
};

template <typename T>
inline constexpr auto extract_type_deduction_dependencies_v = extract_type_deduction_dependencies<T>::value;


// Which field ids this field's data implies the value of. Only a plain
// top-level len_from_field is invertible: len_from_fields wraps an arbitrary
// callable with no inverse, and a producer inside a maybe_field or a union
// alternative only obligates its target conditionally, which is a verify
// problem rather than a derive one.
template <typename T>
struct extract_unconditional_len_sources {
  static constexpr auto value = dep_vec();
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct extract_unconditional_len_sources<
  field<id, T, size, constraint, bound>
>
{
  static constexpr auto value = dep_vec(as_sv(len_source_of<size_type_of<size>>::value));
};

template <typename T>
inline constexpr auto extract_unconditional_len_sources_v = extract_unconditional_len_sources<T>::value;


// A type_switch discriminant is always derivable: variant index i corresponds
// positionally to case i, so the held alternative determines the value. A
// computed switch input or a ladder is not, since neither can be inverted.
template <typename T>
struct extract_switch_discriminants {
  static constexpr auto value = dep_vec();
};

template <fixed_string id, fixed_string matched_id, typename type_switch>
struct extract_switch_discriminants<
  union_field<id, type<match_field<matched_id>, type_switch>>
>
{
  static constexpr auto value = dep_vec(as_sv(matched_id));
};

template <typename T>
inline constexpr auto extract_switch_discriminants_v = extract_switch_discriminants<T>::value;

template <typename... fields>
struct field_list_metadata {
  template <std::size_t... Is>
  static constexpr auto generate_field_table(std::index_sequence<Is...>) {
    return static_map<sv, field_type_info, max_field_count>(
      {
        {as_sv(fields::field_id), field_type_info(meta::type_id<fields>, Is)}...
      }
    );
  }

  static constexpr auto generate_len_dep_table() {
    return static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_length_dependencies_v<fields>}...
      }
    );
  }

  static constexpr auto generate_parse_dependency_table() {
    return static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_parse_dependencies_v<fields>}...
      }
    );
  }

  static constexpr auto generate_type_deduction_dependency_table() {
    return static_map<sv, static_vector<sv, max_dep_count_per_struct>, max_field_count>(
      {
        {as_sv(fields::field_id), extract_type_deduction_dependencies_v<fields>}...
      }
    );
  }

  // Kept apart rather than concatenated: the two kinds now differ in how
  // operator[] treats them, and a field can legitimately be both, which
  // membership in two lists expresses and a tagged single list does not.
  static constexpr auto generate_length_derived_field_ids() {
    dep_vec sources[sizeof...(fields)] = {dep_vec(extract_unconditional_len_sources_v<fields>)...};
    return remove_duplicates(flatten(sources));
  }

  static constexpr auto generate_discriminant_derived_field_ids() {
    dep_vec sources[sizeof...(fields)] = {dep_vec(extract_switch_discriminants_v<fields>)...};
    return remove_duplicates(flatten(sources));
  }

  static constexpr field_table_t field_table = generate_field_table(std::make_index_sequence<sizeof...(fields)>{});
  static constexpr dependency_table_t length_dependency_table = generate_len_dep_table();
  static constexpr dependency_table_t parse_dependency_table = generate_parse_dependency_table();
  static constexpr dependency_table_t type_deduction_dep_table = generate_type_deduction_dependency_table();
  static constexpr dep_vec length_derived_field_ids = generate_length_derived_field_ids();
  static constexpr dep_vec discriminant_derived_field_ids = generate_discriminant_derived_field_ids();
};

template <auto list_metadata>
constexpr auto lookup_field(sv field_name) -> static_optional<field_type_info> {
  auto field_table = meta::type_of<list_metadata>::field_table;
  return field_table[field_name];
}

template <auto list_metadata>
constexpr auto is_length_derived_field(sv field_name) -> bool {
  for(auto id: meta::type_of<list_metadata>::length_derived_field_ids) {
    if(id == field_name)
      return true;
  }
  return false;
}

template <auto list_metadata>
constexpr auto is_discriminant_derived_field(sv field_name) -> bool {
  for(auto id: meta::type_of<list_metadata>::discriminant_derived_field_ids) {
    if(id == field_name)
      return true;
  }
  return false;
}

// The single source of truth for "derived": the write path asks this general
// question, so its notion of "overwritten, do not accept an assignment" cannot
// drift from the two specific questions operator[] asks.
template <auto list_metadata>
constexpr auto is_derived_field(sv field_name) -> bool {
  return is_length_derived_field<list_metadata>(field_name) ||
         is_discriminant_derived_field<list_metadata>(field_name);
}


constexpr bool is_dependencies_resolved(const field_table_t& field_table, const dependency_table_t& dependency_table) {
  for(auto& entry: dependency_table) {
    auto& [field_name, dependencies] = *entry; 
    auto field_info = field_table[field_name];
    auto field_idx = field_info->occurs_at_idx;
    if(dependency_table.size() > 0) {
      for(auto dep_field: dependencies) {
        auto dep_field_info = field_table[dep_field];
        auto dep_field_idx = dep_field_info->occurs_at_idx;
        if(dep_field_idx > field_idx)
          return false;
      }
    }
  }
  return true;
}


// todo use algorithms over raw loops
template <typename list_metadata>
constexpr bool size_dependencies_resolved() {
  auto field_table = list_metadata::field_table;
  auto length_dependency_table = list_metadata::length_dependency_table;
  return is_dependencies_resolved(field_table, length_dependency_table);
}

template <typename list_metadata>
constexpr bool parse_dependencies_resolved() {
  auto field_table = list_metadata::field_table;
  auto parse_dependency_table = list_metadata::parse_dependency_table;
  return is_dependencies_resolved(field_table, parse_dependency_table);
}

template <typename list_metadata>
constexpr bool type_deduction_dependencies_resolved() {
  auto field_table = list_metadata::field_table;
  auto type_deduction_dep_table = list_metadata::type_deduction_dep_table;
  return is_dependencies_resolved(field_table, type_deduction_dep_table);
}

}

#endif /* _FIELD_LIST_METADATA_HPP_ */

// End field_list/field_list_metadata.hpp

// Begin field_list/field_list.hpp
#ifndef _FIELD_LIST_HPP_
#define _FIELD_LIST_HPP_
 
 
 
 
namespace s2s {

// A field whose value the write path derives from other fields. The name
// carries the reason into the diagnostic when an assignment is rejected.
template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_other_fields =
  is_derived_field<list_metadata>(as_sv(field_accessor::field_id));

// A length target is not part of the user's model of the struct: nobody
// supplies it, and no assignment could keep it honest, since operator[] hands
// back a reference and a container can be mutated through it without passing
// any setter. So it has no operator[] overload at all.
template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_a_length =
  is_length_derived_field<list_metadata>(as_sv(field_accessor::field_id));

template <typename field_accessor, auto list_metadata>
concept field_is_derived_from_a_discriminant =
  is_discriminant_derived_field<list_metadata>(as_sv(field_accessor::field_id));

template <auto list_metadata, typename... fields>
struct struct_field_list_impl : struct_field_list_base, fields... {

  struct_field_list_impl() = default;

  // todo move as_sv to common place
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             (!field_is_derived_from_other_fields<field_accessor, list_metadata>)
  constexpr auto& operator[](field_accessor)  {
    using field_type_ref = meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_ref>(*this).value;
  }

  // A discriminant stays readable on a non-const object but hands back a const
  // reference, so an attempted assignment fails as assign-to-const rather than
  // as a wall of unsatisfied-constraint output from no viable overload. How a
  // caller should reach a variance field's held alternative is unsettled, so
  // hiding discriminants the way length targets are hidden waits on that.
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             field_is_derived_from_a_discriminant<field_accessor, list_metadata>
  constexpr const auto& operator[](field_accessor) {
    using field_type_cref = const meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }

  // Constrained rather than unconditional: without excluding length targets
  // here, `const auto& n = fl["len"_f]` would still reach one through a const
  // object, and hiding it from the mutable overload alone would mean nothing.
  template <
    typename field_accessor,
    auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id))
  >
    requires (field_lookup_res.has_value) &&
             (!field_is_derived_from_a_length<field_accessor, list_metadata>)
  constexpr const auto& operator[](field_accessor) const {
    using field_type_cref = const meta::type_of<field_lookup_res->id>&;
    return static_cast<field_type_cref>(*this).value;
  }
};

// The lookup operator[] performs, without its visibility policy. The library's
// own read path resolves a len_from_field size and feeds user callables by
// reading fields, and both can name a length target — which the user cannot
// see, but which is still on the wire and still has to be read.
template <typename field_accessor, auto list_metadata, typename... fields>
constexpr auto& field_value_of(const struct_field_list_impl<list_metadata, fields...>& field_list) {
  constexpr auto field_lookup_res = lookup_field<list_metadata>(as_sv(field_accessor::field_id));
  static_assert(field_lookup_res.has_value, "no such field in this field list");
  using field_type_cref = const meta::type_of<field_lookup_res->id>&;
  return static_cast<field_type_cref>(field_list).value;
}
} /* namespace s2s */


#endif // _FIELD_LIST_HPP_

// End field_list/field_list.hpp

// Begin field_compute/computation_from_fields_impl.hpp
#ifndef _COMPUTATION_FROM_FIELDS_IMPL_HPP_
#define _COMPUTATION_FROM_FIELDS_IMPL_HPP_
 
using namespace s2s_literals;

namespace s2s {

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
        decltype(field_value_of<field_accessor<req_fields>>(struct_field_list_t{}))...
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


// template <auto callable, typename R, field_name_list Fs>
template <typename T>
struct compute_impl;

// todo: static_vector over fixed_string list?
template <auto callable, typename R, fixed_string... req_fields>
struct compute_impl<compute_t<callable, R, fixed_string_list<req_fields...>>>{
  template <auto metadata, typename... fields>
    requires (can_eval_R_from_fields<
                callable, 
                R,
                struct_field_list_impl<metadata, fields...>,
                fixed_string_list<req_fields...>>)
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& flist) const -> R {
    return callable(field_value_of<field_accessor<req_fields>>(flist)...);
  }
};
} /* namespace s2s */

#endif // _COMPUTATION_FROM_FIELDS_IMPL_HPP_

// End field_compute/computation_from_fields_impl.hpp

// Begin field_size/field_size_deduce.hpp
#ifndef _FIELD_SIZE_DEDUCE_HPP_
#define _FIELD_SIZE_DEDUCE_HPP_
 
 
 
 
 
 
namespace s2s {
// Two more partial specializations of the template declared in
// comptime_field_size_deduce.hpp, with argument lists identical to the
// primary's and told apart only by their constraints.
template <auto size>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct deduce_field_size<size> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& struct_fields) const -> std::size_t {
    // Resolving a len_from_field size reads, by definition, a length target —
    // the one thing operator[] no longer exposes.
    return field_value_of<size_type_of<size>>(struct_fields);
  }
};

template <auto size>
  requires is_computed_size_v<size_type_of<size>>
struct deduce_field_size<size> {
  using field_size_type =
    compute_t<size_type_of<size>::f, std::size_t, std::remove_cvref_t<decltype(size_type_of<size>::req_field_list)>>;

  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& struct_fields) const -> std::size_t {
    return compute_impl<field_size_type>{}(struct_fields);
  }
};
} /* namespace s2s */


#endif // _FIELD_SIZE_DEDUCE_HPP_

// End field_size/field_size_deduce.hpp

// Begin type_deduction/switch/switch.hpp
#ifndef _SWITCH_HPP_
#define _SWITCH_HPP_
 
 
namespace s2s {
template <match_case_like... cases>
  requires (sizeof...(cases) > 0)
struct type_switch {
  // todo possibly unused
  using variant = variant_from_type_conditions_v<cases...>;
  using sizes = size_choices_from_type_conditions_v<cases...>;
};
} /* namespace s2s */


#endif // _SWITCH_HPP_

// End type_deduction/switch/switch.hpp

// Begin type_deduction/switch/switch_traits.hpp
#ifndef _SWITCH_TRAITS_HPP_
#define _SWITCH_TRAITS_HPP_
 
 
namespace s2s {
template <typename T>
struct is_type_switch;

template <typename T>
struct is_type_switch {
  static constexpr bool res = false;
};

template <match_case_like case_head, match_case_like... case_tail>
struct is_type_switch<type_switch<case_head, case_tail...>> {
  static constexpr bool res = true;
};

template <typename T>
static constexpr bool is_type_switch_v = is_type_switch<T>::res;

template <typename T>
concept type_switch_like = is_type_switch_v<T>;

} /* namespace s2s */


#endif // _SWITCH_TRAITS_HPP_

// End type_deduction/switch/switch_traits.hpp

// Begin type_deduction/if_else_ladder/ladder_traits.hpp
#ifndef _LADDER_TRAITS_HPP_
#define _LADDER_TRAITS_HPP_
 
namespace s2s {
template <typename T>
struct is_type_if_else;

template <typename T>
struct is_type_if_else {
  static constexpr bool res = false;
};

template <branch_like branch_head, branch_like... branch_tail>
struct is_type_if_else<type_if_else<branch_head, branch_tail...>> {
  static constexpr bool res = true;
};

template <typename T>
static constexpr bool is_type_if_else_v = is_type_if_else<T>::res;

template <typename T>
concept type_if_else_like = is_type_if_else_v<T>;
} /* namespace s2s */

#endif // _LADDER_TRAITS_HPP_

// End type_deduction/if_else_ladder/ladder_traits.hpp

// Begin type_deduction/type/type_deduction_traits.hpp
#ifndef _TYPE_DEDUCTION_TRAITS_HPP_
#define _TYPE_DEDUCTION_TRAITS_HPP_
 
 
 
namespace s2s {

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


template <typename T>
struct is_type_deduction;

template <>
struct is_type_deduction<no_type_deduction> {
  static constexpr bool res = true;
};

template <typename eval_expression, typename _switch>
struct is_type_deduction<type<eval_expression, _switch>> {
  static constexpr bool res = 
    is_compute_like_v<eval_expression> && 
    type_switch_like<_switch>;
};

template <fixed_string id, typename _switch>
struct is_type_deduction<type<match_field<id>, _switch>> {
  static constexpr bool res = type_switch_like<_switch>;
};

template <typename ladder>
struct is_type_deduction<type<ladder>> {
  static constexpr bool res = type_if_else_like<ladder>;
};

template <typename T>
struct is_type_deduction {
  static constexpr bool res = false;
};

template <typename T>
static constexpr bool is_type_deduction_v = is_type_deduction<T>::res;

template <typename T>
concept type_deduction_like = is_type_deduction_v<T>;
} /* namespace s2s */


#endif // _TYPE_DEDUCTION_TRAITS_HPP_

// End type_deduction/type/type_deduction_traits.hpp

// Begin type_deduction/type/type_deduction_metafunctions.hpp
#ifndef _TYPE_DEDUCTION_METAFUNCTIONS_HPP_
#define _TYPE_DEDUCTION_METAFUNCTIONS_HPP_
 
 
 
 
 
 
namespace s2s {
template <typename... type_tags>
struct extract_type_from_tags {
  static constexpr auto type_tag_count = sizeof...(type_tags);
  using type_id_vec = static_vector<meta::type_identifier, type_tag_count>;
  static constexpr auto value = type_id_vec(meta::type_id<typename type_tags::type>...);
};

template <typename T>
struct extract_field_choices;

template <
  fixed_string matched_id, 
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_field_choices<
  type<
    match_field<matched_id>, 
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value = extract_type_from_tags<type_tags...>::value;
};

template <
  auto callable, typename R, typename field_name_list,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_field_choices<
  type<
    compute_t<callable, R, field_name_list>, 
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value = extract_type_from_tags<type_tags...>::value;
};

template <
  auto... callables, typename... field_name_lists, typename... type_tags
>
struct extract_field_choices<
  type<
    type_if_else<
      branch<compute_t<callables, bool, field_name_lists>, type_tags>...
    >
  >
>
{
  static constexpr auto value = extract_type_from_tags<type_tags...>::value;
};

template <std::size_t N>
constexpr bool has_unique_field_choices(const s2s::static_vector<meta::type_identifier, N>& type_id_list) {
  static_set<meta::type_identifier, N> type_id_set(type_id_list);
  return equal_ranges(type_id_list, type_id_set);
}


// has_unique_field_choices enforces unique case *types*, which is what makes
// alternative-to-index inversion well-defined. It says nothing about the case
// *values*, and duplicates there break round-trip silently: writing the second
// alternative emits value v, and reading v back selects the first case that
// matches it. Only the switch forms carry values; a ladder has none.
template <typename T>
struct extract_match_values {
  static constexpr auto value = static_vector<std::size_t, 1>();
};

template <
  fixed_string matched_id,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_match_values<
  type<
    match_field<matched_id>,
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value =
    static_vector<std::size_t, sizeof...(match_values)>(static_cast<std::size_t>(match_values)...);
};

template <
  auto callable, typename R, typename field_name_list,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct extract_match_values<
  type<
    compute_t<callable, R, field_name_list>,
    type_switch<
      match_case<match_values, type_tags>...
    >
  >
>
{
  static constexpr auto value =
    static_vector<std::size_t, sizeof...(match_values)>(static_cast<std::size_t>(match_values)...);
};

template <std::size_t N>
constexpr bool has_unique_match_values(const s2s::static_vector<std::size_t, N>& match_value_list) {
  static_set<std::size_t, N> match_value_set(match_value_list);
  return equal_ranges(match_value_list, match_value_set);
}
}

#endif /* _TYPE_DEDUCTION_METAFUNCTIONS_HPP_ */

// End type_deduction/type/type_deduction_metafunctions.hpp

// Begin api/field_descriptors.hpp
#ifndef _FIELD_DESCRIPTORS_HPP_
#define _FIELD_DESCRIPTORS_HPP_
 
 
 
 
 
 
 
 
namespace s2s {
struct always_true {
  // const: compute_impl invokes the callable as a const NTTP, so without this
  // always_present fails to compile in either direction.
  constexpr auto operator()() const -> bool {
    return true;
  }
};

using always_present = eval_bool_from_fields<always_true{}>;

template <auto size, typename field_type>
concept field_fits_to_underlying_type = deduce_field_size<size>{}() <= sizeof(field_type);

// The trailing options of a descriptor are an unordered pack: a size, a
// constraint, either, or neither. Classification is by the option's *type*,
// which is also why these concepts take it first — a placeholder constraint
// `field_option_like<T> auto... opts` substitutes decltype(opt) as the first
// argument, so a value-parameterised concept could not be used this way at all.
template <typename S, typename T>
concept size_option_like = fixed_size_like<S>    ||
                           variable_size_like<S> ||
                           size_dont_care_like<S> ||
                           selectable_size_like<S>;

template <typename C, typename T>
concept constraint_option_like = requires (const C& c, const T& v) {
  { c(v) } -> std::same_as<bool>;
};

template <typename O, typename T>
concept field_option_like = size_option_like<O, T> || constraint_option_like<O, T>;

// A bound is meaningful only where wire input drives the allocation, so only
// the three container descriptors admit one. Everywhere else max_bytes fails
// the per-element placeholder constraint exactly as an unrecognised entry
// does — the relationship "a bound needs a variable size" is hoisted into
// *which descriptor* accepts it rather than checked between two pack entries,
// which keeps classification per-element and keeps the diagnostic readable.
template <typename B, typename T>
concept bound_pack_option_like = bound_option_like<B>;

template <typename O, typename T>
concept boundable_field_option_like = size_option_like<O, T>       ||
                                      constraint_option_like<O, T> ||
                                      bound_pack_option_like<O, T>;

// Applied per element rather than as a fold in a requires-clause: a fold names
// the fold and dumps the whole pack, while this isolates the offending entry
// and prints both things it could have been.
template <typename T, auto... opts>
inline constexpr std::size_t size_option_count =
  (0u + ... + (size_option_like<size_type_of<opts>, T> ? 1u : 0u));

template <typename T, auto... opts>
inline constexpr std::size_t constraint_option_count =
  (0u + ... + (constraint_option_like<size_type_of<opts>, T> ? 1u : 0u));

template <typename T, auto... opts>
inline constexpr std::size_t bound_option_count =
  (0u + ... + (bound_pack_option_like<size_type_of<opts>, T> ? 1u : 0u));

// A concept can reject an entry it cannot classify but cannot count how many
// classified the same way, so duplicates are the one sanctioned static_assert.
template <typename T, auto... opts>
struct pack_options {
  static_assert(size_option_count<T, opts...> <= 1,
                "a field takes at most one size option");
  static_assert(constraint_option_count<T, opts...> <= 1,
                "a field takes at most one constraint option");
  static_assert(bound_option_count<T, opts...> <= 1,
                "a field takes at most one max_bytes option");
};

template <typename T, auto... opts>
struct size_in_pack {
  static constexpr auto value = byte_count{sizeof(T)};
};

template <typename T, auto head, auto... tail>
struct size_in_pack<T, head, tail...> {
  static constexpr auto value = [] {
    if constexpr(size_option_like<size_type_of<head>, T>)
      return head;
    else
      return size_in_pack<T, tail...>::value;
  }();
};

template <typename T, auto... opts>
struct constraint_in_pack {
  static constexpr auto value = no_constraint<T>{};
};

template <typename T, auto head, auto... tail>
struct constraint_in_pack<T, head, tail...> {
  static constexpr auto value = [] {
    if constexpr(constraint_option_like<size_type_of<head>, T>)
      return head;
    else
      return constraint_in_pack<T, tail...>::value;
  }();
};

template <typename T, auto... opts>
struct bound_in_pack {
  static constexpr auto value = use_default_bound;
};

template <typename T, auto head, auto... tail>
struct bound_in_pack<T, head, tail...> {
  static constexpr auto value = [] {
    if constexpr(bound_pack_option_like<size_type_of<head>, T>)
      return head;
    else
      return bound_in_pack<T, tail...>::value;
  }();
};

// Order independence is exactly this scan; nothing else is needed. Deriving
// from pack_options is what instantiates it, so the duplicate assertions fire.
template <typename T, auto... opts>
struct resolved_options : pack_options<T, opts...> {
  static constexpr auto size = size_in_pack<T, opts...>::value;
  static constexpr auto constraint = constraint_in_pack<T, opts...>::value;
  static constexpr auto bound = bound_in_pack<T, opts...>::value;
};

template <typename T, auto... opts>
inline constexpr auto size_of_pack = resolved_options<T, opts...>::size;

template <typename T, auto... opts>
inline constexpr auto constraint_of_pack = resolved_options<T, opts...>::constraint;

template <typename T, auto... opts>
inline constexpr auto bound_of_pack = resolved_options<T, opts...>::bound;

template <fixed_string id, integral T, field_option_like<T> auto... opts>
  requires field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
using basic_field = field<id, T, size_of_pack<T, opts...>, constraint_of_pack<T, opts...>>;

template <fixed_string id, field_containable T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
using fixed_array_field =
  field<id, std::array<T, N>, byte_count{N * sizeof(T)}, constraint_of_pack<std::array<T, N>, opts...>>;

template <fixed_string id, field_list_like T, std::size_t N,
          constraint_option_like<std::array<T, N>> auto... opts>
using array_of_records =
  field<id, std::array<T, N>, size_dont_care, constraint_of_pack<std::array<T, N>, opts...>>;

template <fixed_string id, std::size_t N, constraint_option_like<fixed_string<N>> auto... opts>
using fixed_string_field =
  field<id, fixed_string<N>, byte_count{N + 1}, constraint_of_pack<fixed_string<N>, opts...>>;

template <fixed_string id, field_containable T, std::size_t N,
          constraint_option_like<T[N]> auto... opts>
using c_arr_field = field<id, T[N], byte_count{N * sizeof(T)}, constraint_of_pack<T[N], opts...>>;

template <fixed_string id, std::size_t N, constraint_option_like<char[N + 1]> auto... opts>
using c_str_field =
  field<id, char[N + 1], byte_count{N * sizeof(char) + 1}, constraint_of_pack<char[N + 1], opts...>>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, byte_count{N}, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, byte_count{expected.size() + 1}, eq{expected}>;

template <fixed_string id, integral T, auto size, auto expected>
  requires fixed_size_like<size_type_of<size>>
using magic_number = field<id, T, size, eq{expected}>;

// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
using vec_field =
  field<id, std::vector<T>, size_of_pack<std::vector<T>, opts...>,
        constraint_of_pack<std::vector<T>, opts...>,
        bound_of_pack<std::vector<T>, opts...>>;

template <fixed_string id, field_list_like T, boundable_field_option_like<std::vector<T>> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::vector<T>, opts...>>>
using vector_of_records =
  field<id, std::vector<T>, size_of_pack<std::vector<T>, opts...>,
        constraint_of_pack<std::vector<T>, opts...>,
        bound_of_pack<std::vector<T>, opts...>>;

// todo check if this will work for all char types like wstring
template <fixed_string id, boundable_field_option_like<std::string> auto... opts>
  requires variable_size_like<size_type_of<size_of_pack<std::string, opts...>>>
using str_field =
  field<id, std::string, size_of_pack<std::string, opts...>,
        constraint_of_pack<std::string, opts...>,
        bound_of_pack<std::string, opts...>>;

template <fixed_string id, field_list_like T, constraint_option_like<T> auto... opts>
using struct_field = field<id, T, size_dont_care, constraint_of_pack<T, opts...>>;

template <no_variance_field_like base_field, typename present_only_if>
  requires is_eval_bool_from_fields_v<present_only_if>
using maybe = maybe_field<base_field, present_only_if>;


template <fixed_string id, type_deduction_like type_deducer>
  requires (has_unique_field_choices(extract_field_choices<type_deducer>::value)) &&
           (has_unique_match_values(extract_match_values<type_deducer>::value))
using variance = union_field<id, type_deducer>;

} /* namespace s2s */

#endif /* _FIELD_DESCRIPTORS_HPP_ */

// End api/field_descriptors.hpp

// Begin field/field_traits.hpp
#ifndef _FIELD_TRAITS_HPP_
#define _FIELD_TRAITS_HPP_
 
 
 
namespace s2s {
template <typename T>
struct is_fixed_sized_field;

// Specialization for field with fixed_size_like size
template <fixed_string id, field_containable T, auto size, auto constraint_on_value, auto bound>
  requires fixed_size_like<size_type_of<size>>
struct is_fixed_sized_field<field<id, T, size, constraint_on_value, bound>> {
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

template <fixed_string id, field_list_like T, std::size_t N, auto size, auto constraint_on_value, auto bound>
struct is_array_of_record_field<field<id, std::array<T, N>, size, constraint_on_value, bound>> {
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
template <fixed_string id, variable_sized_buffer_like T, auto size, auto constraint_on_value, auto bound>
  requires variable_size_like<size_type_of<size>>
struct is_variable_sized_field<field<id, T, size, constraint_on_value, bound>> {
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

template <fixed_string id, field_list_like T, auto size, auto constraint_on_value, auto bound>
struct is_vector_of_record_field<field<id, std::vector<T>, size, constraint_on_value, bound>> {
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
template <fixed_string id, field_list_like T, auto size, auto constraint_on_value, auto bound>
struct is_struct_field<field<id, T, size, constraint_on_value, bound>> {
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
          auto size, 
          auto constraint, 
          auto bound,
          typename present_only_if, 
          typename optional>
struct is_optional_field<
    maybe_field<field<id, T, size, constraint, bound>, present_only_if, optional>
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

template <fixed_string id, typename type_deducer>
struct is_union_field<
    union_field<id, type_deducer>
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

template <typename T>
concept field_like = fixed_sized_field_like<T> || 
                     variable_sized_field_like<T> ||
                     array_of_record_field_like<T> ||
                     vector_of_record_field_like<T> ||
                     struct_field_like<T> || 
                     optional_field_like<T> || 
                     union_field_like<T>;
} /* namespace s2s */

#endif /*_FIELD_TRAITS_HPP_*/

// End field/field_traits.hpp

// Begin api/struct_field_list.hpp
#ifndef _STRUCT_FIELD_LIST_HPP_
#define _STRUCT_FIELD_LIST_HPP_
 
namespace s2s {
template <typename... fields>
concept all_field_like = (field_like<fields> && ...);

template <std::size_t N>
constexpr bool are_field_ids_unique(const std::array<std::string_view, N>& field_id_list) {
  static_set<std::string_view, N> field_id_set(field_id_list);
  return equal_ranges(field_id_list, field_id_set);
}

template <std::size_t N>
constexpr auto as_sv(const fixed_string<N>& str) {
  return std::string_view{str.data()};
}

template <typename... fields>
concept has_unique_field_ids = are_field_ids_unique(std::array{as_sv(fields::field_id)...});

template <typename metadata>
struct dependency_check {
  static constexpr bool size_dependencies_ok = size_dependencies_resolved<metadata>();
  static constexpr bool parse_dependencies_ok = parse_dependencies_resolved<metadata>();
  static constexpr bool type_dependencies_ok = type_deduction_dependencies_resolved<metadata>();

  static_assert(size_dependencies_ok, "Size dependencies not resolved");
  static_assert(parse_dependencies_ok, "Parse dependencies not resolved");
  static_assert(type_dependencies_ok, "Type deduction dependencies not resolved");

  static constexpr bool all_dependencies_ok = 
    size_dependencies_ok && 
    parse_dependencies_ok && 
    type_dependencies_ok;
};

template <typename metadata>
concept all_dependencies_resolved = dependency_check<metadata>::all_dependencies_ok;

template <typename... fields>
  requires (all_dependencies_resolved<field_list_metadata<fields...>>)
struct create_struct_field_list {
  using metadata = field_list_metadata<fields...>;
  static constexpr auto metadata_v = meta::type_id<metadata>;
  using value = struct_field_list_impl<metadata_v, fields...>;
};

template <typename... fields>
  requires all_field_like<fields...> &&
           has_unique_field_ids<fields...>
using struct_field_list = create_struct_field_list<fields...>::value;
}

#endif /* _STRUCT_FIELD_LIST_HPP_ */


// End api/struct_field_list.hpp

// Begin field/field_metafunctions.hpp
#ifndef _FIELD_METAFUNCTIONS_HPP_
#define _FIELD_METAFUNCTIONS_HPP_
 
 
 
namespace s2s {
struct not_a_field;

template <typename T>
struct extract_type_from_field;

template <fixed_string id, typename field_type, auto size, auto constraint, auto bound>
struct extract_type_from_field<field<id, field_type, size, constraint, bound>> {
  using type = field_type;
};

template <typename T>
struct extract_type_from_field {
  using type = not_a_field;
};

template <typename T>
using extract_type_from_field_v = typename extract_type_from_field<T>::type;
} /* namespace s2s */

#endif // _FIELD_METAFUNCTIONS_HPP_

// End field/field_metafunctions.hpp

// Begin type_deduction/if_else_ladder/ladder_impl.hpp
#ifndef _LADDER_IMPL_HPP_
#define _LADDER_IMPL_HPP_
 
namespace s2s {
template <typename ladder>
struct evaluate_ladder;

template <std::size_t idx, typename branch>
struct evalaute_ladder_impl;

template <std::size_t idx, typename branch>
struct evalaute_ladder_impl {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& field_list) const -> 
    type_deduction_idx
  {
    if(compute_impl<typename branch::expression>{}(field_list)) 
      return idx;
    return std::nullopt;
  }
};

template <typename... branches>
struct evaluate_ladder_helper {
  template <auto metadata, typename... fields, std::size_t... idx>
  constexpr auto operator()(
    const struct_field_list_impl<metadata, fields...>& field_list, 
    const std::index_sequence<idx...>&) const 
  -> type_deduction_idx
  {
    type_deduction_idx pipeline_seed = std::nullopt;
    return (
      pipeline_seed |
      ... |
      [&]() { return evalaute_ladder_impl<idx, branches>{}(field_list); }
    );
  }
};


template <typename... branches>
struct evaluate_ladder<type_if_else<branches...>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& field_list) const -> 
    type_deduction_res
  {
    auto res = evaluate_ladder_helper<branches...>{}(
      field_list,
      std::make_index_sequence<sizeof...(branches)>{}
    );
    if(!res)
      return std::unexpected(error_reason::type_deduction_failure);
    return std::expected<std::size_t, error_reason>(*res);
  }
};
} /* namespace s2s */

#endif // _LADDER_IMPL_HPP_

// End type_deduction/if_else_ladder/ladder_impl.hpp

// Begin type_deduction/switch/switch_impl.hpp
#ifndef _SWITCH_IMPL_HPP_
#define _SWITCH_IMPL_HPP_
 
namespace s2s {

// todo check if case and eval result match in terms of types
template <std::size_t idx, typename match_case>
struct evaluate_switch_impl {
  constexpr auto operator()(const auto& v) const -> type_deduction_idx
  {
    if(v == match_case::value) return idx;
    else return std::nullopt;
  }
};

template <typename... match_cases>
struct evaluate_switch_helper {
  template <std::size_t... idx>
  constexpr auto operator()(const auto& v, const std::index_sequence<idx...>&) const 
  -> type_deduction_idx 
  {
    type_deduction_idx pipeline_seed = std::nullopt;
    return (
      pipeline_seed |
      ... |
      [&]() { return evaluate_switch_impl<idx, match_cases>{}(v); }
    );
  }
};

template <typename _switch>
struct evaluate_switch;

template <typename... cases>
struct evaluate_switch<type_switch<cases...>> {
  constexpr auto operator()(const auto& v) const -> type_deduction_res
  {
    auto res =
      evaluate_switch_helper<cases...>{}(
        v, std::make_index_sequence<sizeof...(cases)>{}
      );
    if(!res)
      return std::unexpected(error_reason::type_deduction_failure);
    return std::expected<std::size_t, error_reason>(*res);
  } 
};
} /* namespace s2s */


#endif // _SWITCH_IMPL_HPP_

// End type_deduction/switch/switch_impl.hpp

// Begin type_deduction/type/type_impl.hpp
#ifndef _TYPE_IMPL_HPP_
#define _TYPE_IMPL_HPP_
 
namespace s2s {
template <typename... Args>
struct deduce_type;


// todo constraints compute like
template <typename eval_expression, typename _switch>
struct deduce_type<type<eval_expression, _switch>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return evaluate_switch<_switch>{}(compute_impl<eval_expression>{}(sfl)); 
  }
};

template <fixed_string id, typename _switch>
struct deduce_type<type<match_field<id>, _switch>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return evaluate_switch<_switch>{}(sfl[field_accessor<id>{}]); 
  }
};

// todo constraints
template <typename ladder>
struct deduce_type<type<ladder>> {
  template <auto metadata, typename... fields>
  constexpr auto operator()(const struct_field_list_impl<metadata, fields...>& sfl)
    -> std::expected<std::size_t, error_reason> const {
    return evaluate_ladder<ladder>{}(sfl);
  }
};
} /* namespace s2s */


#endif // _TYPE_IMPL_HPP_

// End type_deduction/type/type_impl.hpp

// Begin stream/stream_traits.hpp
#ifndef _STREAM_TRAITS_HPP_
#define _STREAM_TRAITS_HPP_
 
 
 
namespace s2s {

template <typename T>
concept convertible_to_bool = requires(T obj) {
  { obj.operator bool() } -> std::same_as<bool>;
  { !obj } -> std::same_as<bool>;
};

template <typename T>
concept std_read_trait = requires(T obj, char* dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::convertible_to<std::istream&>;
};

template <typename T>
concept std_write_trait = requires(T obj, const char* src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::convertible_to<std::ostream&>;
};

template <typename T>
concept read_trait = requires(T obj, char* dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T>
concept write_trait = requires(T obj, const char* src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T, std::size_t N>
concept constexpr_read_trait = requires(T obj, std::array<char, N>& dest_mem, std::streamsize size_to_read) {
  { obj.read(dest_mem, size_to_read) } -> std::same_as<T&>;
};

template <typename T, std::size_t N>
concept constexpr_write_trait = requires(T obj, const std::array<char, N>& src_mem, std::size_t size_to_read) {
  { obj.write(src_mem, size_to_read) } -> std::same_as<T&>;
};

// todo add operator bool, seekg, tellg, fail, bad, eof/s constaint
struct constexpr_stream{};

template <typename T>
concept identified_as_constexpr_stream = std::is_base_of_v<constexpr_stream, T>;

template <typename T>
concept writeable = std_write_trait<T> || write_trait<T>;

template <typename T>
concept readable = std_read_trait<T> || read_trait<T>;

// todo improve this to be more typesafe
template <typename T>
concept input_stream_like = (identified_as_constexpr_stream<T> || readable<T>) && convertible_to_bool<T>;

template <typename T>
concept output_stream_like = (identified_as_constexpr_stream<T> || writeable<T>) && convertible_to_bool<T>;
}

#endif /* _STREAM_TRAITS_HPP_ */

// End stream/stream_traits.hpp

// Begin lib/memory/address_manip.hpp
#ifndef _ADDRESS_MANIP_HPP_
#define _ADDRESS_MANIP_HPP_
 
 
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

// End lib/memory/address_manip.hpp

// Begin stream/byte_order.hpp
#ifndef _BYTE_ORDER_HPP_
#define _BYTE_ORDER_HPP_
 
namespace s2s {
// Single-byte elements have no byte order, and fixed_string exposes no
// iterators, so both fall through untouched. Shared by the read and write
// directions, which is why it lives here rather than beside either one.
template <buffer_like T>
constexpr auto byteswap_elements(T& obj) -> void {
  if constexpr(!fixed_string_like<T>) {
    for(auto& elem: obj) {
      // Multi-dimensional aggregates nest, so descend until the scalars.
      if constexpr(buffer_like<std::remove_reference_t<decltype(elem)>>)
        byteswap_elements(elem);
      else if constexpr(sizeof(elem) > 1)
        elem = std::byteswap(elem);
    }
  }
}


enum cast_endianness {
  host = 0,
  foreign = 1
};


template <std::endian endianness>
constexpr cast_endianness deduce_byte_order() {
  if constexpr(std::endian::native == endianness)
    return cast_endianness::host;
  else if constexpr(std::endian::native != endianness)
    return cast_endianness::foreign;
}
} /* namespace s2s */

#endif // _BYTE_ORDER_HPP_

// End stream/byte_order.hpp

// Begin field_read/read_impl.hpp
#ifndef _READ_IMPL_HPP_
#define _READ_IMPL_HPP_
 
 
 
 
 
namespace s2s {
// The gate every wire-driven allocation passes through.
//
// Phrased as a division so `len * sizeof(element)` is never evaluated on an
// unvalidated `len` — not even to test it. `ceiling / sizeof(element)` is a
// compile-time constant, so this costs one comparison. It also makes the
// overflow test and the bound test the same test: at SIZE_MAX it fires exactly
// when the product would wrap, and at a lower ceiling it fires when the product
// would wrap or exceed that ceiling.
//
// No guard for a zero element size: C++ has no complete type of size zero.
template <typename element, std::size_t ceiling = std::numeric_limits<std::size_t>::max()>
constexpr auto checked_byte_count(std::size_t len) -> std::expected<std::size_t, error_reason> {
  if(len > ceiling / sizeof(element))
    return std::unexpected(error_reason::excessive_length);
  return len * sizeof(element);
}

template <typename T, identified_as_constexpr_stream stream>
constexpr auto read_native_impl(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  auto as_byte_buffer_rep = as_byte_buffer<stream>(obj);
  if(!s.read(as_byte_buffer_rep, size_to_read)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  if constexpr(std::is_array_v<T>) {
    // bit_cast cannot yield a C array — it returns by value. The buffer goes
    // to the equivalent std::array instead and the elements are assigned.
    using element = std::remove_extent_t<T>;
    auto as_std_array = std::bit_cast<std::array<element, std::extent_v<T>>>(as_byte_buffer_rep);
    for(std::size_t idx = 0; idx < std::extent_v<T>; ++idx)
      obj[idx] = as_std_array[idx];
  } else {
    obj = std::bit_cast<T>(as_byte_buffer_rep);
  }
  return {};
}

template <typename T, readable stream>
constexpr auto read_native_impl(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  if(!s.read(byte_addressof<stream>(obj), size_to_read)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  return {};
}

template <constant_sized_like T, input_stream_like stream>
constexpr auto read_native(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  return read_native_impl(s, obj, size_to_read);   
}

template <std::size_t ceiling = default_max_bytes, variable_sized_buffer_like T, input_stream_like stream>
constexpr auto read_native(stream& s, T& obj, std::size_t len_to_read) -> rw_result {
  // Above the resize *and* above the constexpr branch: no allocation
  // proportional to an unvalidated length happens in either mode, and the
  // reject path stays reachable during constant evaluation.
  //
  // The check lives here, at the allocation, rather than at the caller — that
  // makes read_native locally sound for every caller instead of leaving a
  // wrapping multiply justified by a caller-side invariant.
  const auto byte_count = checked_byte_count<typename T::value_type, ceiling>(len_to_read);
  if(!byte_count)
    return std::unexpected(byte_count.error());

  obj.resize(len_to_read);
  if constexpr(identified_as_constexpr_stream<stream>) {
    // Mirrors write_native: a vector cannot be bit_cast during constant
    // evaluation, so fill it element by element.
    for(auto& elem: obj) {
      auto res = read_native_impl(s, elem, sizeof(elem));
      if(!res)
        return res;
    }
    return {};
  } else {
    return read_native_impl(s, obj, *byte_count);
  }
}

template <trivial T, input_stream_like stream>
constexpr auto read_foreign_scalar(stream& s, T& obj, std::size_t size_to_read) -> rw_result {
  auto res = read_native_impl(s, obj, size_to_read);
  if(res) {
    // todo rollout byteswap if freestanding compiler doesnt provide one
    obj = std::byteswap(obj);
    return {};
  }
  return res;
}

template <std::size_t ceiling = default_max_bytes, buffer_like T, input_stream_like stream>
constexpr auto read_foreign_buffer(stream& s, T& obj, std::size_t len_to_read) -> rw_result {
  auto res = [&] {
    if constexpr(variable_sized_buffer_like<T>)
      return read_native<ceiling>(s, obj, len_to_read);
    else
      return read_native(s, obj, len_to_read);
  }();
  if(res) {
    byteswap_elements(obj);
    return {};
  }
  return res;
}

// The ceiling is a defaulted NTTP after endianness so the existing call sites
// keep compiling verbatim; only the resizing overload of read_native is handed
// one, since the constant-sized overload has nothing to bound.
template <std::endian endianness, std::size_t ceiling = default_max_bytes,
          typename T, input_stream_like stream>
constexpr auto read_impl(stream& s, T& obj, std::size_t N) -> rw_result {
  auto constexpr byte_order = deduce_byte_order<endianness>();
  if constexpr(byte_order == cast_endianness::host) {
    if constexpr(variable_sized_buffer_like<T>)
      return read_native<ceiling>(s, obj, N);
    else
      return read_native(s, obj, N);
  } else if constexpr(byte_order == cast_endianness::foreign) {
    if constexpr(trivial<T>) {
      return read_foreign_scalar(s, obj, N);
    } else if constexpr(buffer_like<T>) {
      return read_foreign_buffer<ceiling>(s, obj, N);
    }
  }
}


} /* namespace s2s */

#endif /* _READ_IMPL_HPP_ */

// End field_read/read_impl.hpp

// Begin field_read/field_reader.hpp
#ifndef _FIELD_READER_HPP_
#define _FIELD_READER_HPP_
 
 
 
namespace s2s {
template <typename F, typename L>
struct read_field;

template <fixed_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;

  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list) {}
  
  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> rw_result {
    constexpr auto field_size = T::field_size;
    constexpr auto size_to_read = deduce_field_size<field_size>{}();
    return read_impl<endianness>(s, field.value, size_to_read);
  }
};


template <variable_sized_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> rw_result {
    constexpr auto field_size = T::field_size;
    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    return read_impl<endianness, bound_in_bytes<T::field_bound>>(s, field.value, len_to_read);
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
  static constexpr auto size = size_dont_care;
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
  static constexpr auto size = size_dont_care;
  static constexpr auto constraint = no_constraint<vector_elem_type>{};

  using res = field<field_id, vector_elem_type, size, constraint>;
};

template <typename T>
using create_field_from_vector_of_records_v = create_field_from_vector_of_records<T>::res;

template <typename T, typename F, typename E>
struct read_buffer_of_records {
  T& field;
  F& field_list;
    std::size_t len_to_read;

  constexpr read_buffer_of_records(T& field, F& field_list, std::size_t len_to_read)
    : field(field), field_list(field_list), len_to_read(len_to_read) {}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> rw_result {
    for(std::size_t count = 0; count < len_to_read; ++count) {
      E elem;
      auto reader = read_field<E, F>(elem, field_list);
      auto res = reader.template read<endianness, stream>(s);
      if(!res) 
        return std::unexpected(res.error());
      field.value[count] = std::move(elem.value);
    }
    return {};
  }
};

template <array_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> rw_result {
    using array_type = typename T::field_type;
    using array_element_field = create_field_from_array_of_records_v<T>;
    using read_impl_t = read_buffer_of_records<T, F, array_element_field>;

    constexpr auto array_len = extract_size_from_array_v<array_type>;
    auto reader = read_impl_t(field, field_list, array_len);
    auto res = reader.template read<endianness>(s);
    return res;
  }
};


template <vector_of_record_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> rw_result {
    using vector_element_field = create_field_from_vector_of_records_v<T>;
    constexpr auto field_size = T::field_size;
    using read_impl_t = read_buffer_of_records<T, F, vector_element_field>;

    auto len_to_read = deduce_field_size<field_size>{}(field_list);
    // Validate the footprint before reserving it. The product is discarded on
    // purpose: a record's wire size is not sizeof(record), so only the
    // predicate is shared with the buffer site, not the byte count.
    const auto footprint =
      checked_byte_count<typename T::field_type::value_type, bound_in_bytes<T::field_bound>>(len_to_read);
    if(!footprint)
      return std::unexpected(footprint.error());

    field.value.resize(len_to_read);
    auto reader = read_impl_t(field, field_list, len_to_read);
    auto res = reader.template read<endianness>(s);
    return res;
  }
};


template <typename F, typename stream, auto endianness>
struct struct_cast_impl;

template <struct_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list)
    : field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) const -> rw_result {
    using field_list_t = extract_type_from_field_v<T>;
    auto res = struct_cast_impl<field_list_t, stream, endianness>{}(s);
    if(!res) {
      auto err = res.error();
      return std::unexpected(err.failure_reason);
    }
    field.value = *res;
    return {};
  }
};



template <optional_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list): 
    field(field), field_list(field_list){}
  
  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> rw_result {
    if(!compute_impl<typename T::field_presence_checker>{}(field_list)) {
      field.value = std::nullopt;
      return {};
    }
    using field_base_type_t = typename T::field_base_type;
    field_base_type_t base_field{};
    read_field<field_base_type_t, F> reader(base_field, field_list);
    auto res = reader.template read<endianness>(s);
    if(!res) 
      return std::unexpected(res.error());
    field.value = base_field.value;
    return {};
  }
};


// Helper function to read bytes into the variant
template<std::size_t idx, typename T, typename F, typename V>
struct read_variant_impl {
  V& variant;
  F& field_list;
  std::size_t idx_r;

  constexpr explicit read_variant_impl(
    V& variant, 
    F& field_list,
    std::size_t idx_r) :
      variant(variant), field_list(field_list), idx_r(idx_r) {}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> rw_result {
    if (idx_r != idx) 
      return {};

    T field;
    auto reader = read_field<T, F>(field, field_list);
    auto res = reader.template read<endianness, stream>(s);
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
  std::size_t idx_r;
  
  constexpr read_variant_helper(T& field, F& field_list, std::size_t idx_r) 
    : field(field), field_list(field_list), idx_r(idx_r) {}
  
  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> rw_result {
    rw_result pipeline_seed{};
    return (
      pipeline_seed |
      ... | 
      [&]() {
        auto reader_impl = read_variant_impl<idx, fields, F, typename T::field_type>(field.value, field_list, idx_r);
        return reader_impl.template read<endianness>(s);
      }
    );
  }
};


template <union_field_like T, field_list_like F>
struct read_field<T, F> {
  T& field;
  F& field_list;
  
  constexpr read_field(T& field, F& field_list): 
    field(field), field_list(field_list){}

  template <auto endianness, typename stream>
  constexpr auto read(stream& s) -> rw_result {
    using type_deduction_guide = typename T::type_deduction_guide;
    using field_choices = typename T::field_choices;
    constexpr auto max_type_index = T::variant_size;

    auto type_index_deducer = deduce_type<type_deduction_guide>();
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
    auto field_reader = read_helper_t(field, field_list, idx_r);
    auto field_read_res = field_reader.template read<endianness, stream>(s);
    if(!field_read_res)
      return std::unexpected(field_read_res.error());
    return {};
  }
};
} /* namespace s2s */

#endif // _FIELD_READER_HPP_

// End field_read/field_reader.hpp

// Begin cast/struct_cast_impl.hpp
#ifndef _STRUCT_CAST_IMPL_HPP_
#define _STRUCT_CAST_IMPL_HPP_
 
namespace s2s {

template <typename F, typename stream, auto endianness>
struct struct_cast_impl;

template <auto metadata, typename... fields, typename stream, auto endianness>
struct struct_cast_impl<struct_field_list_impl<metadata, fields...>, stream, endianness> {
  using S = struct_field_list_impl<metadata, fields...>;
  using R = std::expected<S, cast_error>;

  constexpr auto operator()(stream& s) -> R {
    S field_list;
    cast_result pipeline_seed{};
    auto res = (
      pipeline_seed |
      ... |
      [&]() -> cast_result {
        auto& field = static_cast<fields&>(field_list);
        auto reader = read_field<fields, S>(field, field_list);
        auto read_res = reader.template read<endianness>(s);
        // Short circuit the remaining pipeline since read failed for current field
        if(!read_res) {
          auto field_name = std::string_view{fields::field_id.data()};
          auto err = read_res.error();
          auto validation_err = cast_error{err, field_name};
          return std::unexpected(validation_err);
        }
        bool field_validation_res = fields::constraint_checker(field.value);
        if(!field_validation_res) {
          auto field_name = std::string_view{fields::field_id.data()};
          auto err = error_reason::validation_failure;
          auto validation_err = cast_error{err, field_name};
          return std::unexpected(validation_err);
        }
        // Both reading and validating went well
        return {};
      }
    );
    return res ? R(field_list) : std::unexpected(res.error());
  }
};

} /* namespace s2s */

#endif // _STRUCT_CAST_IMPL_HPP_

// End cast/struct_cast_impl.hpp

// Begin api/struct_cast.hpp
#ifndef _STRUCT_CAST_HPP_
#define _STRUCT_CAST_HPP_

// status: split to cast and cast impl
 
 
namespace s2s {
template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_le(stream& s) -> std::expected<T, cast_error> {
  return struct_cast_impl<T, stream, std::endian::little>{}(s);
}

template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_be(stream& s) -> std::expected<T, cast_error> {
  return struct_cast_impl<T, stream, std::endian::big>{}(s);
}
} /* namespace s2s */

#endif // _STRUCT_CAST_HPP_

// End api/struct_cast.hpp

// Begin field_write/derived_value.hpp
#ifndef _DERIVED_VALUE_HPP_
#define _DERIVED_VALUE_HPP_
 
namespace s2s {
// An obligation: a field whose data implies what an earlier field's value has
// to be. Only len_from_field is invertible — len_from_fields wraps an
// arbitrary callable, so it obligates nothing and its sources stay writable.
template <typename T>
struct len_obligation {
  static constexpr bool present = false;
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct len_obligation<field<id, T, size, constraint, bound>> {
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source_of<size_type_of<size>>::value);
};

template <typename producer, typename target>
constexpr auto obligates() -> bool {
  if constexpr(len_obligation<producer>::present)
    return len_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

// The other invertible dependency. variant_from_type_conditions_v builds the
// variant in case order, so index i is case i positionally — the inverse is
// total, with no search and no ambiguity, and has_unique_match_values keeps
// it that way.
template <typename T>
struct discriminant_obligation {
  static constexpr bool present = false;
};

template <
  fixed_string id, fixed_string matched_id,
  template<typename...> typename type_switch,
  auto... match_values, typename... type_tags
>
struct discriminant_obligation<
  union_field<
    id,
    type<match_field<matched_id>, type_switch<match_case<match_values, type_tags>...>>
  >
>
{
  static constexpr bool present = true;
  static constexpr sv target = as_sv(matched_id);

  static constexpr auto value_at(std::size_t alternative_index) -> std::size_t {
    constexpr std::size_t values[] = {static_cast<std::size_t>(match_values)...};
    return values[alternative_index];
  }
};

template <typename producer, typename target>
constexpr auto discriminant_obligates() -> bool {
  if constexpr(discriminant_obligation<producer>::present)
    return discriminant_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

// A producer sitting inside a maybe_field obligates its target only when the
// optional is actually present. That is why such a target cannot be derived —
// there may be nothing to derive from — but when the producer is present its
// container still has to match whatever goes on the wire.
template <typename T>
struct conditional_len_obligation {
  static constexpr bool present = false;
};

template <fixed_string id, typename T, auto size, auto constraint, auto bound,
          typename present_only_if, typename optional>
  requires (variable_size_like<size_type_of<size>> && !is_computed_size_v<size_type_of<size>>)
struct conditional_len_obligation<
  maybe_field<
    field<id, T, size, constraint, bound>,
    present_only_if,
    optional
  >
>
{
  static constexpr bool present = true;
  static constexpr sv target = as_sv(len_source_of<size_type_of<size>>::value);
};

template <typename producer, typename target>
constexpr auto conditionally_obligates() -> bool {
  if constexpr(conditional_len_obligation<producer>::present)
    return conditional_len_obligation<producer>::target == as_sv(target::field_id);
  else
    return false;
}

// The other conditional shape: a union alternative that is itself a
// length-prefixed container obligates its length only while that alternative
// is the one held.
template <typename producer, typename target, typename idx_seq>
struct union_len_obligation {
  static constexpr bool present = false;
};

template <typename target, typename... choices, std::size_t... idx>
struct union_len_obligation<field_choice_list<choices...>, target, std::index_sequence<idx...>> {
  static constexpr bool present = (... || obligates<choices, target>());

  template <typename V>
  static constexpr auto agrees(const V& variant, std::size_t value_on_the_wire) -> bool {
    bool ok{true};
    (([&] {
      if constexpr(obligates<choices, target>()) {
        if(variant.index() == idx && std::get<idx>(variant).size() != value_on_the_wire)
          ok = false;
      }
    }()), ...);
    return ok;
  }
};

template <typename producer, typename target>
struct union_len_obligation_of {
  static constexpr bool present = false;
};

template <fixed_string id, typename type_deducer, typename target>
struct union_len_obligation_of<union_field<id, type_deducer>, target> {
  using field = union_field<id, type_deducer>;
  using resolved = union_len_obligation<
    typename field::field_choices,
    target,
    std::make_index_sequence<field::variant_size>
  >;
  static constexpr bool present = resolved::present;
};


template <typename target, typename F>
struct has_conditional_len_obligation {
  static constexpr bool value = false;
};

template <typename target, auto metadata, typename... fields>
struct has_conditional_len_obligation<target, struct_field_list_impl<metadata, fields...>> {
  static constexpr bool value =
    (... || (conditionally_obligates<fields, target>() ||
             union_len_obligation_of<fields, target>::present));
};

template <typename target, typename F>
inline constexpr bool has_conditional_len_obligation_v =
  has_conditional_len_obligation<target, F>::value;


template <typename target, typename F>
struct verify_conditional_len;

template <typename target, auto metadata, typename... fields>
struct verify_conditional_len<target, struct_field_list_impl<metadata, fields...>> {
  using S = struct_field_list_impl<metadata, fields...>;

  // An obligation counts only when its producer will actually be written.
  // Presence is judged by the same predicate the reader will apply, so a
  // predicate that disagrees with has_value() is left to fail at the optional
  // itself rather than being reported here as a length contradiction.
  constexpr auto operator()(const S& field_list, std::size_t value_on_the_wire) const
    -> rw_result
  {
    bool agreed{true};

    (([&] {
      const auto& producer = static_cast<const fields&>(field_list);
      if constexpr(conditionally_obligates<fields, target>()) {
        const auto is_active =
          producer.value.has_value() &&
          compute_impl<typename fields::field_presence_checker>{}(field_list);
        if(is_active && producer.value->size() != value_on_the_wire)
          agreed = false;
      } else if constexpr(union_len_obligation_of<fields, target>::present) {
        using resolved = typename union_len_obligation_of<fields, target>::resolved;
        if(!resolved::agrees(producer.value, value_on_the_wire))
          agreed = false;
      }
    }()), ...);

    if(!agreed)
      return std::unexpected(error_reason::found_contradicting_length);
    return {};
  }
};


template <typename target, typename F>
struct is_derived_target {
  static constexpr bool value = false;
};

// Deliberately not an independent scan: operator[] rejects assignment to
// exactly the fields the writer overwrites, and two scans would drift.
template <typename target, auto metadata, typename... fields>
struct is_derived_target<target, struct_field_list_impl<metadata, fields...>> {
  static constexpr bool value = is_derived_field<metadata>(as_sv(target::field_id));

  // A length slot is always a fixed-width field. Without this a schema
  // pointing len_from_field at a variable-sized field fails deep inside
  // deduce_field_size with an incomplete-type error instead.
  static_assert(!value || fixed_sized_field_like<target>,
                "a derived length field must be a fixed-sized field");
};

template <typename target, typename F>
inline constexpr bool is_derived_target_v = is_derived_target<target, F>::value;


template <typename target, typename F>
struct derive_value;

// The obligations for a target all live in fields that come after it, and
// is_dependencies_resolved has already rejected any schema where that is not
// so — hence the forward scan over the whole pack.
template <typename target, auto metadata, typename... fields>
struct derive_value<target, struct_field_list_impl<metadata, fields...>> {
  using S = struct_field_list_impl<metadata, fields...>;
  using field_type = typename target::field_type;

  constexpr auto operator()(const S& field_list) const
    -> std::expected<field_type, error_reason>
  {
    std::size_t derived{0};
    bool seen{false};
    bool agreed{true};

    auto record = [&](std::size_t implied) {
      if(seen && implied != derived)
        agreed = false;
      derived = implied;
      seen = true;
    };

    (([&] {
      const auto& producer = static_cast<const fields&>(field_list);
      if constexpr(obligates<fields, target>())
        record(producer.value.size());
      else if constexpr(discriminant_obligates<fields, target>())
        record(discriminant_obligation<fields>::value_at(producer.value.index()));
    }()), ...);

    // Two distinct failures: the dependents contradict one another, or they
    // agree on a value the declared slot cannot hold.
    if(!agreed)
      return std::unexpected(error_reason::found_contradicting_length);
    if(!fits_declared_width(derived))
      return std::unexpected(error_reason::validation_failure);
    return static_cast<field_type>(derived);
  }

private:
  // The declared width, not sizeof(field_type): a u32 slot declared
  // 2_B puts two bytes on the wire, and a length needing
  // three must fail rather than reach the stream truncated.
  static constexpr auto declared_width = deduce_field_size<target::field_size>{}();

  static constexpr auto fits_declared_width(std::size_t v) -> bool {
    if constexpr(declared_width < sizeof(std::size_t)) {
      if((v >> (declared_width * 8)) != 0)
        return false;
    }
    return static_cast<std::size_t>(static_cast<field_type>(v)) == v;
  }
};
} /* namespace s2s */

#endif // _DERIVED_VALUE_HPP_

// End field_write/derived_value.hpp

// Begin field_write/write_impl.hpp
#ifndef _WRITE_IMPL_HPP_
#define _WRITE_IMPL_HPP_
 
 
namespace s2s {
template <typename T, identified_as_constexpr_stream stream>
constexpr auto write_native_impl(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  auto as_byte_buffer_rep = as_byte_buffer<stream>(obj);
  if(!s.write(as_byte_buffer_rep, size_to_write)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  return {};
}

template <typename T, writeable stream>
constexpr auto write_native_impl(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  if(!s.write(const_byte_addressof<stream>(obj), size_to_write)) {
    return std::unexpected(error_reason::buffer_exhaustion);
  }
  return {};
}

template <constant_sized_like T, output_stream_like stream>
constexpr auto write_native(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  return write_native_impl(s, obj, size_to_write);
}

template <variable_sized_buffer_like T, output_stream_like stream>
constexpr auto write_native(stream& s, const T& obj, std::size_t len_to_write) -> rw_result {
  if constexpr(identified_as_constexpr_stream<stream>) {
    // A vector's bytes cannot be bit_cast out of it during constant
    // evaluation, so the constexpr stream takes one element at a time.
    for(std::size_t idx = 0; idx < len_to_write; ++idx) {
      auto res = write_native_impl(s, obj[idx], sizeof(obj[idx]));
      if(!res)
        return res;
    }
    return {};
  } else {
    return write_native_impl(s, obj, len_to_write * sizeof(T{}[0]));
  }
}

template <trivial T, output_stream_like stream>
constexpr auto write_foreign_scalar(stream& s, const T& obj, std::size_t size_to_write) -> rw_result {
  // The source is const and belongs to the caller, so the swap lands in a
  // stack temporary rather than mutating it in place as the read path does.
  T swapped = std::byteswap(obj);
  return write_native_impl(s, swapped, size_to_write);
}

// The source buffer is const, so unlike the read direction there is nothing to
// swap in place. Writing element by element keeps the swap in a scalar
// temporary rather than staging a byteswapped copy of the whole buffer, which
// for a vector would mean allocating.
template <buffer_like T, output_stream_like stream>
constexpr auto write_foreign_buffer(stream& s, const T& obj, std::size_t len_to_write) -> rw_result {
  if constexpr(fixed_string_like<T>) {
    return write_native(s, obj, len_to_write);
  } else {
    for(const auto& elem: obj) {
      auto res = [&] {
        // Multi-dimensional aggregates nest, so descend until the scalars.
        if constexpr(buffer_like<std::remove_cvref_t<decltype(elem)>>)
          return write_foreign_buffer(s, elem, sizeof(elem));
        else if constexpr(sizeof(elem) > 1)
          return write_foreign_scalar(s, elem, sizeof(elem));
        else
          return write_native_impl(s, elem, sizeof(elem));
      }();
      if(!res)
        return res;
    }
    return {};
  }
}

template <std::endian endianness, typename T, output_stream_like stream>
constexpr auto write_impl(stream& s, const T& obj, std::size_t N) -> rw_result {
  auto constexpr byte_order = deduce_byte_order<endianness>();
  if constexpr(byte_order == cast_endianness::host) {
    return write_native(s, obj, N);
  } else if constexpr(byte_order == cast_endianness::foreign) {
    if constexpr(trivial<T>) {
      return write_foreign_scalar(s, obj, N);
    } else if constexpr(buffer_like<T>) {
      return write_foreign_buffer(s, obj, N);
    }
  }
}
} /* namespace s2s */

#endif // _WRITE_IMPL_HPP_

// End field_write/write_impl.hpp

// Begin field_write/field_writer.hpp
#ifndef _FIELD_WRITER_HPP_
#define _FIELD_WRITER_HPP_
 
 
 
namespace s2s {
template <typename F, typename L>
struct write_field;

template <fixed_sized_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    constexpr auto field_size = T::field_size;
    constexpr auto size_to_write = deduce_field_size<field_size>{}();
    if constexpr(is_derived_target_v<T, F>) {
      // The stored value is ignored, so the constraint has to be checked
      // against the derived one — stream_cast_impl skips this field.
      auto derived = derive_value<T, F>{}(field_list);
      if(!derived)
        return std::unexpected(derived.error());
      if(!T::constraint_checker(*derived))
        return std::unexpected(error_reason::validation_failure);
      return verify_then_write<endianness>(s, *derived, size_to_write);
    } else {
      return verify_then_write<endianness>(s, value, size_to_write);
    }
  }

private:
  // Conditional producers cannot make this field derived, so whatever value
  // reaches this point — derived or stored — still has to satisfy every
  // obligation that is currently active.
  template <auto endianness, typename stream>
  constexpr auto verify_then_write(
    stream& s, const typename T::field_type& v, std::size_t size_to_write) const -> rw_result
  {
    if constexpr(has_conditional_len_obligation_v<T, F>) {
      auto res = verify_conditional_len<T, F>{}(field_list, static_cast<std::size_t>(v));
      if(!res)
        return res;
    }
    return write_impl<endianness>(s, v, size_to_write);
  }
};

template <variable_sized_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    constexpr auto field_size = T::field_size;
    if constexpr(is_computed_size_v<size_type_of<field_size>>) {
      // An arbitrary N-ary callable has no inverse, so its source fields stay
      // ordinary data and the size they imply can only be checked against the
      // container, never used to repair it.
      if(deduce_field_size<field_size>{}(field_list) != value.size())
        return std::unexpected(error_reason::found_contradicting_length);
    }
    // For a len_from_field size there is nothing to check: the length slot was
    // derived from this very container, so the container is the authority.
    return write_impl<endianness>(s, value, value.size());
  }
};


template <typename F, typename stream, auto endianness>
struct stream_cast_impl;

// The one seam where the error representation narrows. A nested list names
// its own failing field, but rw_result carries no name and the outer fold
// re-attaches the outer field's id, so failed_at ends up naming the outermost
// record field. read_field<struct_field_like> does exactly the same.
template <field_list_like L, auto endianness, typename stream>
constexpr auto write_nested(stream& s, const L& nested) -> rw_result {
  auto res = stream_cast_impl<L, stream, endianness>{}(s, nested);
  if(!res)
    return std::unexpected(res.error().failure_reason);
  return {};
}

template <struct_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using field_list_t = extract_type_from_field_v<T>;
    return write_nested<field_list_t, endianness>(s, value);
  }
};

template <array_of_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using array_type = typename T::field_type;
    using element_t = extract_type_from_array_v<array_type>;
    constexpr auto array_len = extract_size_from_array_v<array_type>;

    for(std::size_t count = 0; count < array_len; ++count) {
      auto res = write_nested<element_t, endianness>(s, value[count]);
      if(!res)
        return res;
    }
    return {};
  }
};

template <vector_of_record_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using vector_type = typename T::field_type;
    using element_t = extract_type_from_vec_t<vector_type>;
    constexpr auto field_size = T::field_size;

    if constexpr(is_computed_size_v<size_type_of<field_size>>) {
      if(deduce_field_size<field_size>{}(field_list) != value.size())
        return std::unexpected(error_reason::found_contradicting_length);
    }
    for(const auto& record: value) {
      auto res = write_nested<element_t, endianness>(s, record);
      if(!res)
        return res;
    }
    return {};
  }
};

template <optional_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    // Presence is a predicate over siblings, not a stored flag, so there is
    // nothing to derive here — only to check that the struct agrees with what
    // the reader will conclude from the very same sibling bytes.
    const auto should_be_present =
      compute_impl<typename T::field_presence_checker>{}(field_list);
    if(should_be_present != value.has_value())
      return std::unexpected(error_reason::validation_failure);
    if(!should_be_present)
      return {};

    using base_t = typename T::field_base_type;
    // maybe_field rewrites the base field's constraint to no_constraint over
    // the optional, so the fold cannot run it and this is the only place it
    // reaches the engaged value.
    if(!base_t::constraint_checker(*value))
      return std::unexpected(error_reason::validation_failure);
    return write_field<base_t, F>(*value, field_list).template write<endianness>(s);
  }
};


template <std::size_t idx, typename E, typename F, typename V>
struct write_variant_impl {
  const V& variant;
  const F& field_list;

  constexpr write_variant_impl(const V& variant, const F& field_list)
    : variant(variant), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    if(variant.index() != idx)
      return {};
    return write_field<E, F>(std::get<idx>(variant), field_list).template write<endianness>(s);
  }
};

template <typename F, typename field_choices, typename idx_seq>
struct write_variant_helper;

template <typename F, typename... choices, std::size_t... idx>
struct write_variant_helper<F, field_choice_list<choices...>, std::index_sequence<idx...>> {
  template <auto endianness, typename stream, typename V>
  static constexpr auto write(stream& s, const V& variant, const F& field_list) -> rw_result {
    rw_result pipeline_seed{};
    return (
      pipeline_seed |
      ... |
      [&]() {
        return write_variant_impl<idx, choices, F, V>(variant, field_list)
                 .template write<endianness>(s);
      }
    );
  }
};

template <union_field_like T, field_list_like F>
struct write_field<T, F> {
  const typename T::field_type& value;
  const F& field_list;

  constexpr write_field(const typename T::field_type& value, const F& field_list)
    : value(value), field_list(field_list) {}

  template <auto endianness, typename stream>
  constexpr auto write(stream& s) const -> rw_result {
    using guide = typename T::type_deduction_guide;

    // Exactly the unions whose discriminant is derivable need no check here:
    // the discriminant came from this alternative, so agreement is structural.
    // A computed switch input or a ladder cannot be inverted, so the held
    // alternative can only be checked against what the reader will conclude
    // from the same sibling bytes.
    if constexpr(!discriminant_obligation<T>::present) {
      auto deduced = deduce_type<guide>{}(field_list);
      if(!deduced)
        return std::unexpected(deduced.error());
      if(*deduced != value.index())
        return std::unexpected(error_reason::validation_failure);
    }

    using helper = write_variant_helper<
      F,
      typename T::field_choices,
      std::make_index_sequence<T::variant_size>
    >;
    return helper::template write<endianness>(s, value, field_list);
  }
};
} /* namespace s2s */

#endif // _FIELD_WRITER_HPP_

// End field_write/field_writer.hpp

// Begin cast/stream_cast_impl.hpp
#ifndef _STREAM_CAST_IMPL_HPP_
#define _STREAM_CAST_IMPL_HPP_
 
namespace s2s {

template <typename F, typename stream, auto endianness>
struct stream_cast_impl;

template <auto metadata, typename... fields, typename stream, auto endianness>
struct stream_cast_impl<struct_field_list_impl<metadata, fields...>, stream, endianness> {
  using S = struct_field_list_impl<metadata, fields...>;

  constexpr auto operator()(stream& s, const S& field_list) -> cast_result {
    cast_result pipeline_seed{};
    return (
      pipeline_seed |
      ... |
      [&]() -> cast_result {
        const auto& field = static_cast<const fields&>(field_list);
        // Validated before writing, not after: a struct that fails its own
        // constraint would otherwise emit bytes that cannot be read back.
        // Derived fields are the exception — their stored value is ignored,
        // so write_field checks the constraint against the derived one.
        if constexpr(!is_derived_target_v<fields, S>) {
          if(!fields::constraint_checker(field.value)) {
            auto field_name = std::string_view{fields::field_id.data()};
            return std::unexpected(cast_error{error_reason::validation_failure, field_name});
          }
        }
        auto writer = write_field<fields, S>(field.value, field_list);
        auto write_res = writer.template write<endianness>(s);
        if(!write_res) {
          auto field_name = std::string_view{fields::field_id.data()};
          return std::unexpected(cast_error{write_res.error(), field_name});
        }
        return {};
      }
    );
  }
};

} /* namespace s2s */

#endif // _STREAM_CAST_IMPL_HPP_

// End cast/stream_cast_impl.hpp

// Begin api/stream_cast.hpp
#ifndef _STREAM_CAST_HPP_
#define _STREAM_CAST_HPP_
 
namespace s2s {
template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_le(stream& s, const T& obj) -> cast_result {
  return stream_cast_impl<T, stream, std::endian::little>{}(s, obj);
}

template <field_list_like T, output_stream_like stream>
[[nodiscard]] constexpr auto stream_cast_be(stream& s, const T& obj) -> cast_result {
  return stream_cast_impl<T, stream, std::endian::big>{}(s, obj);
}
} /* namespace s2s */

#endif // _STREAM_CAST_HPP_

// End api/stream_cast.hpp

// Begin s2s.hpp
#ifndef STRUCT_CAST_HPP
#define STRUCT_CAST_HPP
 
 
 
 
 
 
 
 
 
 
 
 
 
#endif // STRUCT_CAST_HPP

// End s2s.hpp
