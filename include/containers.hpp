#ifndef _CONTAINERS_HPP_
#define _CONTAINERS_HPP_


#include <cstdint>
#include <utility>
#include "algorithms.hpp"


template <typename T, std::size_t N>
class static_vector {
public:
  constexpr static_vector() = default;
  template <typename... Args>
  constexpr static_vector(Args&&... entries) {
    (push_back(entries),...);
  }
  constexpr auto push_back(const T& value) { 
    values[vec_size] = value; 
    vec_size++;
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { return values[i]; }
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
    (insert(entries),...);
  }
  constexpr static_set(const static_vector<T, N>& vec) {
    for(auto value: vec) { insert(value); }
  }
  constexpr static_set(const std::array<T, N>& vec) {
    for(auto value: vec) { insert(value); }
  }
  constexpr auto insert(const T& value) { 
    if(find_index(value, *this) == set.size()) {
      set.push_back(value);
    }
  }
  [[nodiscard]] constexpr const auto& operator[](std::size_t i) const { return set[i]; }
  [[nodiscard]] constexpr auto begin() const { return set.begin(); }
  [[nodiscard]] constexpr auto end() const { return set.end(); }
  [[nodiscard]] constexpr auto size() const { return set.size(); }
  [[nodiscard]] constexpr auto empty() const { return not set.size(); }
  [[nodiscard]] constexpr auto capacity() const { return N; }

private:
  static_vector<T, N> set{};
};

template <typename T>
constexpr void swap(T& a, T& b) {
  T temp{a};
  a = b;
  b = temp;
}


template <typename Key, typename Value>
struct Node {
  std::pair<Key, Value> element;
  constexpr Node() = default;
  constexpr Node(Key key, Value value): element(key, value) {}

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
    auto key_index = find_index_if([&key](auto t){ return t.element.first == key; }, map);
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
  constexpr auto generate_map(const std::pair<Key, Value> (&entries)[C], std::index_sequence<Is...>){
    static_vector<Node<Key, Value>, N> m;
    (m.push_back(Node(entries[Is].first, entries[Is].second)), ...);
    // std::ranges::sort(map/*, compare*/);
    return m;
  }
};


constexpr auto generate_test_map() -> static_map<std::string_view, std::string_view, 5> {
  return static_map<std::string_view, std::string_view, 5> (
    { {"hello", "world"}, {"foo", "bar"} }
  );
}
constexpr auto map = generate_test_map();
static_assert(map["hello"] == "world");
static_assert(map["world"] == std::nullopt);

constexpr auto generate_test_set() -> static_set<std::string_view, 5> {
  // static_set<std::string_view, 5> set;
  // set.insert("hello");
  // set.insert("world");
  // set.insert("foo");
  // set.insert("bar");
  // set.insert("bar");

  static_set<std::string_view, 5> set("hello", "world", "foo", "bar", "bar");
  return set;
}
//
// constexpr auto generate_test_vector() -> static_vector<std::string_view, 5> {
//   static_vector<std::string_view, 5> vec;
//   vec.push_back("hello");
//   vec.push_back("world");
//   vec.push_back("foo");
//   vec.push_back("bar");
//
//   return vec;
// }
//
static constexpr auto set = generate_test_set();
// static constexpr auto vec = generate_test_vector();
// static_assert(set[0] == std::string_view{"hello"});
static_assert(set.size() == 4);
// static_assert(equal_ranges(set, vec));
// static constexpr static_set<std::string_view, 5> s(vec);
// static_assert(s[0] == std::string_view{"hello"});
// static_assert(s.size() == 4);

#endif /* _CONTAINERS_HPP_ */
