#ifndef _CONTAINERS_HPP_
#define _CONTAINERS_HPP_


#include <cstdint>
#include "algorithms.hpp"

template <typename T, std::size_t N>
struct static_vector {
  constexpr static_vector() = default;
  constexpr auto push_back(const T& value) { 
    values[vec_size] = value; 
    vec_size++;
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { return values[i]; }
  [[nodiscard]] constexpr auto begin() const { return &values[0]; }
  [[nodiscard]] constexpr auto end() const { return &values[0] + vec_size; }
  [[nodiscard]] constexpr auto size() const { return vec_size; }
  [[nodiscard]] constexpr auto empty() const { return not vec_size; }
  [[nodiscard]] constexpr auto capacity() const { return N; }

  T values[N]{};
  std::size_t vec_size{0};
};


template <typename T, std::size_t N>
struct static_set {
  constexpr static_set() = default;
  constexpr static_set(const static_vector<T, N>& vec) {
    for(auto value: vec) { push_back(value); }
  }
  constexpr static_set(const std::array<T, N>& vec) {
    for(auto value: vec) { push_back(value); }
  }
  constexpr auto push_back(const T& value) { 
    if(find_index(value, *this) == set.size()) {
      set.push_back(value);
    }
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { return set[i]; }
  [[nodiscard]] constexpr auto begin() const { return set.begin(); }
  [[nodiscard]] constexpr auto end() const { return set.end(); }
  [[nodiscard]] constexpr auto size() const { return set.size(); }
  [[nodiscard]] constexpr auto empty() const { return not set.size(); }
  [[nodiscard]] constexpr auto capacity() const { return N; }

  static_vector<T, N> set{};
};

// constexpr auto generate_test_set() -> static_set<std::string_view, 5> {
//   static_set<std::string_view, 5> set;
//   set.push_back("hello");
//   set.push_back("world");
//   set.push_back("foo");
//   set.push_back("bar");
//   set.push_back("bar");
//
//   return set;
// }
//
// constexpr auto generate_test_vector() -> static_vector<std::string_view, 5> {
//   static_vector<std::string_view, 5> vec;
//   vec.push_back("hello");
//   vec.push_back("world");
//   vec.push_back("foo");
//   vec.push_back("bar");
//
//   return vec;
// }
//
// static constexpr auto set = generate_test_set();
// static constexpr auto vec = generate_test_vector();
// static_assert(set[0] == std::string_view{"hello"});
// static_assert(set.size() == 4);
// static_assert(equal_ranges(set, vec));
// static constexpr static_set<std::string_view, 5> s(vec);
// static_assert(s[0] == std::string_view{"hello"});
// static_assert(s.size() == 4);

#endif /* _CONTAINERS_HPP_ */
