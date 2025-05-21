#ifndef _STATIC_MAP_HPP_
#define _STATIC_MAP_HPP_


#include <cstdint>
#include <utility>
#include <optional>
#include <functional>
#include "static_vector.hpp"
#include "../algorithms/algorithms.hpp"


namespace s2s {
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
}

#endif /* _STATIC_MAP_HPP_ */
