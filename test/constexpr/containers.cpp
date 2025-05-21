#include <iostream>
#include "../include/s2s.hpp"

// template <typename Key, typename Value, std::size_t N, std::size_t C>
// constexpr auto map(const std::pair<Key, Value> (&entries)[C]) -> static_map_impl<Key, Value, N> {
//   return static_map_impl<Key, Value, N>(entries);
// }
// using N = Node<std::string_view, std::string_view>;
// static_assert(N("hello", "world") < N("world", "nothing"));
//
// constexpr auto generate_test_map() -> static_map<std::string_view, std::string_view, 5> {
//   return static_map<std::string_view, std::string_view, 5> (
//     { 
//       {"hello", "world"}, {"foo", "bar"},
//       {"world", "nothing"}, {"arc", "not-arc"},
//       {"algebra", "math"}
//     }
//   );
// }
// constexpr auto map = generate_test_map();
// static_assert(*map["hello"] == "world");
// static_assert(*map["world"] == "nothing");

// constexpr auto generate_test_set() -> static_set<std::string_view, 5> {
  // static_set<std::string_view, 5> set;
  // set.insert("hello");
  // set.insert("world");
  // set.insert("foo");
  // set.insert("bar");
  // set.insert("bar");

  // static_set<std::string_view, 5> set("hello", "world", "foo", "bar", "bar");
  // return set;
// }
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
// static constexpr auto set = generate_test_set();
// static constexpr auto vec = generate_test_vector();
// static_assert(set[0] == std::string_view{"hello"});
// static_assert(set.size() == 4);
// static_assert(equal_ranges(set, vec));
// static constexpr static_set<std::string_view, 5> s(vec);
// static_assert(s[0] == std::string_view{"hello"});
// static_assert(s.size() == 4);
//

// constexpr static_optional<int> i = 42;
// template <auto val>
// struct k {};
//
// k<i> k_inst;
//
// static_assert(static_nullopt == static_nullopt);


auto main(void) -> int {
  auto map = static_map<std::string_view, std::string_view, 6> (
    { {"hello", "world"}, {"foo", "bar"},
      {"world", "nothing"}, {"arc", "not-arc"},
      {"algebra", "math"}, {"foo", "bar"}
    }
  );

  for(auto& entry: map) {
    std::cout << entry->first <<  " " << entry->second << '\n';
  }

  for(auto& entry: map) {
    auto& [a, b] = *entry;
    std::cout << entry->first <<  " " << entry->second << '\n';
  }

  auto vec = static_vector<int, 5>(1, 9, 5, 3, 0);
  sort_ranges(vec, std::less<int>{});
  for(auto e: vec) { std::cout << e << '\n'; }

  auto s_vec = static_vector<std::string_view, 5>(
    "hello",
    "pope",
    "chocolate",
    "iota",
    "maracuja"
  );
  sort_ranges(s_vec, std::less<>{});
  for(auto e: s_vec) { std::cout << e << '\n'; }

  return 0;
}
