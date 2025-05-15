#ifndef _ALGORITHMS_HPP_
#define _ALGORITHMS_HPP_


#include <ranges>
#include "fixed_string.hpp"

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
