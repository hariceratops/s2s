#ifndef _STATIC_VECTOR_HPP_
#define _STATIC_VECTOR_HPP_


#include <cstdint>


namespace s2s {
template <typename T, std::size_t N>
class static_vector {
public:
  constexpr static_vector() = default;
  template <typename... Args>
  constexpr static_vector(Args&&... entries) {
    (push_back(entries), ...);
  }
  constexpr auto push_back(const T& value) { 
    values[vec_size] = value; 
    vec_size++;
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { 
    return values[i]; 
  }
  [[nodiscard]] constexpr auto& operator[](std::size_t i) { 
    return values[i]; 
  }
  [[nodiscard]] constexpr auto begin() const { return &values[0]; }
  [[nodiscard]] constexpr auto end() const { return &values[0] + vec_size; }
  [[nodiscard]] constexpr auto size() const { return vec_size; }
  [[nodiscard]] constexpr auto empty() const { return not vec_size; }
  [[nodiscard]] constexpr auto capacity() const { return N; }

private:
  T values[N]{};
  std::size_t vec_size{0};
};
}

#endif /* _STATIC_VECTOR_HPP_ */
