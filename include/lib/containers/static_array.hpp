#ifndef _STATIC_ARRAY_HPP_
#define _STATIC_ARRAY_HPP_


#include <cstdint>

namespace s2s {
template <typename T, std::size_t N>
class static_array {
public:
  // to facilitate usage as non-type template parameter
  T values[N]{};

  template <typename... Args>
  constexpr static_array(Args&&... entries) {
    static_assert(sizeof...(Args) == N, "Initializers shall be exactly as the container size");
    std::size_t vec_size{0};
    ([&] {
      values[vec_size] = entries; 
      vec_size++;
    }(), ...);
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { 
    return values[i]; 
  }
  [[nodiscard]] constexpr auto begin() const { return &values[0]; }
  [[nodiscard]] constexpr auto end() const { return &values[0] + N; }
  [[nodiscard]] constexpr auto size() const { return N; }
  [[nodiscard]] constexpr auto empty() const { return not N; }
  [[nodiscard]] constexpr auto capacity() const { return N; }
};
}

#endif /* _STATIC_ARRAY_HPP_ */
