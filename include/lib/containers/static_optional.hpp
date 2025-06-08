#ifndef _STATIC_OPTIONAL_HPP_
#define _STATIC_OPTIONAL_HPP_


namespace s2s {
template <typename T>
struct static_optional {
  T value{};
  bool has_value{false};

  // Constructors
  constexpr static_optional() = default;
  constexpr static_optional(const T& val)
    : value(val), has_value(true) {}
  constexpr static_optional(T&& val)
    : value(std::move(val)), has_value(true) {}

  [[nodiscard]] constexpr bool has() const noexcept { return has_value; }
  [[nodiscard]] constexpr const T& get() const { return value; }
  [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value; }
  [[nodiscard]] constexpr const T& operator*() const { return value; }
  [[nodiscard]] constexpr T& operator*() { return value; }
  [[nodiscard]] constexpr const T* operator->() const { return &value; }
  [[nodiscard]] constexpr T* operator->() { return &value; }
  [[nodiscard]] constexpr auto operator<=>(const static_optional&) const = default;
};

using static_nullopt_t = static_optional<void>;
inline constexpr static_nullopt_t static_nullopt();
}

#endif /* _STATIC_OPTIONAL_HPP_ */
