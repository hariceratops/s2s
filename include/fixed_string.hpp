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
