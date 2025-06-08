#ifndef _STATIC_SET_HPP_
#define _STATIC_SET_HPP_


#include <cstdint>
#include <array>
#include "static_vector.hpp"
#include "../algorithms/algorithms.hpp"


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
