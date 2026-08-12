// Replaces test/constexpr/containers.cpp, whose nine static_asserts were all
// commented out and whose `main` printed the containers to cout without
// checking anything — a registered CTest entry that could not fail. The
// behaviours it printed are asserted here instead.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <cstddef>
#include <functional>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;

int main() {
  "a static_vector holds what it was constructed with"_test = [] constexpr {
    s2s::static_vector<int, 5> vec(1, 9, 5, 3, 0);

    expect(eq(vec.size(), std::size_t{5}));
    expect(eq(vec.capacity(), std::size_t{5}));
    expect(eq(vec.empty(), false));
    expect(eq(vec[0], 1));
    expect(eq(vec[4], 0));
  };

  "a default constructed static_vector is empty but has capacity"_test = [] constexpr {
    s2s::static_vector<int, 5> vec;

    expect(eq(vec.empty(), true));
    expect(eq(vec.size(), std::size_t{0}));
    expect(eq(vec.capacity(), std::size_t{5}));
  };

  "push_back appends in order"_test = [] constexpr {
    s2s::static_vector<int, 3> vec;
    vec.push_back(7);
    vec.push_back(8);

    expect(eq(vec.size(), std::size_t{2}));
    expect(eq(vec[0], 7));
    expect(eq(vec[1], 8));
  };

  // The old file sorted this exact vector and printed it.
  "sort_ranges orders a static_vector of integers"_test = [] constexpr {
    s2s::static_vector<int, 5> vec(1, 9, 5, 3, 0);
    sort_ranges(vec, std::less<int>{});

    expect(eq(vec[0], 0));
    expect(eq(vec[1], 1));
    expect(eq(vec[2], 3));
    expect(eq(vec[3], 5));
    expect(eq(vec[4], 9));
  };

  // Same algorithm over a comparison that is not on integers.
  "sort_ranges orders a static_vector of string views"_test = [] constexpr {
    s2s::static_vector<std::string_view, 5> vec(
      "hello", "pope", "chocolate", "iota", "maracuja");
    sort_ranges(vec, std::less<>{});

    expect(eq(vec[0], std::string_view{"chocolate"}));
    expect(eq(vec[1], std::string_view{"hello"}));
    expect(eq(vec[2], std::string_view{"iota"}));
    expect(eq(vec[3], std::string_view{"maracuja"}));
    expect(eq(vec[4], std::string_view{"pope"}));
  };

  "a static_map looks up the values it was built from"_test = [] constexpr {
    s2s::static_map<std::string_view, std::string_view, 5> map({
      {"hello", "world"}, {"foo", "bar"},
      {"world", "nothing"}, {"arc", "not-arc"},
      {"algebra", "math"}
    });

    expect(eq(*map["hello"], std::string_view{"world"}));
    expect(eq(*map["world"], std::string_view{"nothing"}));
    expect(eq(*map["algebra"], std::string_view{"math"}));
  };

  "a static_map reports a missing key rather than a value"_test = [] constexpr {
    s2s::static_map<std::string_view, std::string_view, 2> map({
      {"hello", "world"}, {"foo", "bar"}
    });

    expect(eq(static_cast<bool>(map["hello"]), true));
    expect(eq(static_cast<bool>(map["nothing"]), false));
  };
}
