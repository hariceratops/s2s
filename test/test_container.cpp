#include <iostream>
#include "../include/s2s.hpp"

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
