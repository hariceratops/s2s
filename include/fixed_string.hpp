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


#endif // _FIXED_STRING_HPP_
