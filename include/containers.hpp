#ifndef _CONTAINERS_HPP_
#define _CONTAINERS_HPP_


#include <cstdint>
#include "algorithms.hpp"


template <typename T, std::size_t N>
struct static_set {
  constexpr static_set() = default;
  constexpr auto push_back(const T& value) { 
    if(find_index(value, *this) == set_size)
      values[set_size++] = value; 
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { return values[i]; }
  [[nodiscard]] constexpr auto begin() const { return &values[0]; }
  [[nodiscard]] constexpr auto end() const { return &values[0] + set_size; }
  [[nodiscard]] constexpr auto size() const { return set_size; }
  [[nodiscard]] constexpr auto empty() const { return not set_size; }
  [[nodiscard]] constexpr auto capacity() const { return N; }

  T values[N]{};
  std::size_t set_size{};
};

#endif /* _CONTAINERS_HPP_ */
