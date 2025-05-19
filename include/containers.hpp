#ifndef _CONTAINERS_HPP_
#define _CONTAINERS_HPP_


#include <cstdint>
#include <utility>
#include "algorithms.hpp"


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


template <typename Key, typename Value>
class Node {
public:
  std::pair<Key, Value> element;

  constexpr Node() = default;
  constexpr Node(Key&& key, Value&& value): element(key, value) {}
  constexpr Node(const Key& key, const Value& value): element(key, value) {}
  constexpr Node(const Node& other): element(other.element) {}
  constexpr Node& operator=(const Node& other) {
    element.first = other.element.first;
    element.second = other.element.second;
    return *this;
  }

  constexpr bool operator<(const Node& rhs) const noexcept {
    return element.first < rhs.element.first;
  }

  constexpr const auto& operator*() const noexcept {
    return element;
  }

  constexpr const auto* operator->() const noexcept {
    return &element;
  }
};


template <typename Key, typename Value, std::size_t N /*, compare? */>
class static_map {
public:
  constexpr static_map() = default;
  constexpr static_map(const std::pair<Key, Value> (&entries)[N]): 
    map(generate_map<N>(entries, std::make_index_sequence<N>{})) {}
  constexpr auto operator[](const Key& key) const  -> std::optional<Value> {
    auto key_index = find_index_if(map, [&key](auto t){ return t.element.first == key; });
    if(key_index != map.size())
      return map[key_index].element.second;
    return std::nullopt;
  }
  [[nodiscard]] constexpr auto begin() const { return map.begin(); }
  [[nodiscard]] constexpr auto end() const { return map.end(); }
  [[nodiscard]] constexpr auto size() const { return map.size(); }
  [[nodiscard]] constexpr auto empty() const { return not map.size(); }
  [[nodiscard]] constexpr auto capacity() const { return N; }

private:
  static_vector<Node<Key, Value>, N> map{};
  template <std::size_t C, std::size_t... Is>
  constexpr auto generate_map(const std::pair<Key, Value> (&entries)[C], std::index_sequence<Is...>) {
    static_vector<Node<Key, Value>, N> m{};
    ([&]() {
      auto key = entries[Is].first;
      auto key_index = find_index_if(m, [&key](auto t){ return t.element.first == key; });
      if(key_index == m.size()) {
        m.push_back(Node(entries[Is].first, entries[Is].second));
        return;
      }
      m[key_index].element.second = entries[Is].second;
    }(), ...);
    sort_ranges(m, std::less<>{});
    return m;
  }
};

#endif /* _CONTAINERS_HPP_ */
