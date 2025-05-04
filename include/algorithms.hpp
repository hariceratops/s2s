#ifndef _ALGORITHMS_HPP_
#define _ALGORITHMS_HPP_


#include <ranges>
#include "fixed_string.hpp"

// todo namespace algorithms
constexpr auto find_index(auto t, const std::ranges::range auto& ts) -> std::size_t {
  for(auto i = 0u; i < ts.size(); ++i) {
    if(ts[i] == t) {
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


static_assert(
  0 == 
  find_index(
    s2s::fixed_string("hello").to_sv(), 
    std::array{s2s::fixed_string("hello").to_sv(), s2s::fixed_string("world").to_sv()})
  );

#endif /* _ALGORITHMS_HPP_ */
