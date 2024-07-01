#ifndef FIELD_CONSTRAINT_HPP
#define FIELD_CONSTRAINT_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cassert>
#include <cstdio>
#include <type_traits>
#include "fixed_string.hpp"


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
