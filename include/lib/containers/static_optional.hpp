#ifndef _STATIC_OPTIONAL_HPP_
#define _STATIC_OPTIONAL_HPP_

#include <optional>

namespace s2s {

struct static_nullopt_t {};
inline constexpr static_nullopt_t nullopt;

template <typename T>
struct static_optional {
  T value{};
  bool has_value{false};

  // Constructors
  constexpr static_optional() = default;
  constexpr static_optional(static_nullopt_t): has_value{false} {};
  constexpr static_optional(const static_nullopt_t&): has_value{false} {};
  constexpr static_optional(const T& val)
    : value(val), has_value(true) {}
  constexpr static_optional(const std::optional<T>& std_opt) {
    if(std_opt) {
      has_value = true;
      value = *std_opt;
    }
  }
  // constexpr static_optional(T&& val)
  //   : value(std::move(val)), has_value(true) {}

  [[nodiscard]] constexpr bool has() const noexcept { return has_value; }
  [[nodiscard]] constexpr const T& get() const { return value; }
  [[nodiscard]] constexpr operator bool() const noexcept { return has_value; }
  [[nodiscard]] constexpr const T& operator*() const { return value; }
  [[nodiscard]] constexpr T& operator*() { return value; }
  [[nodiscard]] constexpr const T* operator->() const { return &value; }
  [[nodiscard]] constexpr T* operator->() { return &value; }
  [[nodiscard]] constexpr auto operator<=>(const static_optional&) const = default;
  [[nodiscard]] constexpr bool operator==(static_nullopt_t&&) const { 
    if(has_value) 
      return true;
    return false; 
  };
};
}

#endif /* _STATIC_OPTIONAL_HPP_ */
