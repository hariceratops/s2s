#ifndef _ALGORITHMS_HPP_
#define _ALGORITHMS_HPP_


#include <ranges>
#include "fixed_string.hpp"

constexpr auto find_index(auto t, const std::ranges::range auto& ts) -> std::size_t {
  for(auto i = 0u; i < ts.size(); ++i) {
    if(ts[i] == t) {
      return i;
    }
  }

  return ts.size();
}


static_assert(
  0 == 
  find_index(
    s2s::fixed_string("hello").to_sv(), 
    std::array{s2s::fixed_string("hello").to_sv(), s2s::fixed_string("world").to_sv()})
  );

#endif /* _ALGORITHMS_HPP_ */
