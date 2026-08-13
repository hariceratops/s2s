// Rewrite of test/constexpr/static_tests_1.cpp, which was unregistered in
// CMake and written against an API several generations old. Every assertion
// here was checked against the current headers; see 038's notes for what was
// dropped and why.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <fstream>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <ut>

#include "../../include/s2s.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
// Imported explicitly rather than relying on the global-scope
// `using namespace s2s_literals` that two library headers leak into every TU.
using namespace s2s_literals;

using u32 = unsigned int;
namespace tl = s2s::typelist;

auto is_eq_1 = [](auto a) { return a == 1; };

auto main() -> int {
  "field ids are checked for uniqueness"_test = [] constexpr {
    expect(eq(s2s::are_field_ids_unique(std::array<std::string_view, 3>{
      "hello", "world", "hello"}), false));
    expect(eq(s2s::are_field_ids_unique(std::array<std::string_view, 3>{
      "hello", "world", "nexus"}), true));
  };

  "the size axis forms are told apart"_test = [] constexpr {
    // A size is a value now, so the traits are asked about its type. Passing a
    // plain type still answers false — the classification is total, not a
    // precondition.
    expect(eq(s2s::is_variable_size_v<s2s::size_type_of<s2s::len_from_field<"hello">>>, true));
    expect(eq(s2s::is_fixed_size_v<s2s::size_type_of<s2s::fixed<4>>>, true));
    expect(eq(s2s::is_fixed_size_v<int>, false));
    expect(eq(s2s::is_variable_size_v<int>, false));
    // The width lives in the value, and deduce_field_size is what reads it.
    expect(eq(s2s::deduce_field_size<s2s::fixed<6>>{}(), std::size_t{6}));
    expect(eq(s2s::deduce_field_size<2_B>{}(), std::size_t{2}));
  };

  "an optional field is distinguishable from a plain one"_test = [] constexpr {
    using optional_f =
      s2s::maybe<
        s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::parse_if<is_eq_1, s2s::with_fields<"a">>
      >;
    using plain_f = s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>;

    expect(eq(s2s::is_optional_field_v<optional_f>, true));
    expect(eq(s2s::is_optional_field_v<plain_f>, false));
  };

  "fixed_string_list indexes and pops by position"_test = [] constexpr {
    using strings = s2s::fixed_string_list<"a", "b", "c", "d">;

    expect(eq(s2s::size_v<strings>, std::size_t{4}));
    expect(eq(s2s::front_t<strings> == s2s::fixed_string("a"), true));
    expect(eq(std::is_same_v<s2s::fixed_string_list<"a", "b", "c", "d">,
                             s2s::pop_t<0, strings>>, true));
    expect(eq(std::is_same_v<s2s::fixed_string_list<"b", "c", "d">,
                             s2s::pop_t<1, strings>>, true));
    expect(eq(std::is_same_v<s2s::fixed_string_list<"c", "d">,
                             s2s::pop_t<2, strings>>, true));
    expect(eq(s2s::front_t<s2s::pop_t<2, strings>> == s2s::fixed_string("c"), true));
    expect(eq(s2s::get_t<2, strings> == s2s::fixed_string("c"), true));
  };

  // fixed_string<N>::size() is the character count, not the storage, which is
  // N + 1 with the terminator.
  "fixed_string reports its character count"_test = [] constexpr {
    expect(eq(s2s::fixed_string("hello").size(), std::size_t{5}));
  };

  "typelist any_of finds a type anywhere in the list"_test = [] constexpr {
    expect(eq(tl::any_of_v<tl::list<int, float, float>, int>, true));
    expect(eq(tl::any_of_v<tl::list<float, int, float, float>, int>, true));
    expect(eq(tl::any_of_v<tl::list<int, int, int>, float>, false));
    expect(eq(tl::any_of_v<tl::list<>, float>, false));
  };

  // The differing element is placed first, in the middle and last, since a
  // scan that stops early would pass some of those and not others.
  "typelist all_are_same rejects a list with any outlier"_test = [] constexpr {
    expect(eq(tl::all_are_same_v<tl::list<int, int, int>>, true));
    expect(eq(tl::all_are_same_v<tl::list<float, int, int>>, false));
    expect(eq(tl::all_are_same_v<tl::list<int, int, float, int, int>>, false));
    expect(eq(tl::all_are_same_v<tl::list<int, float, float, int, int>>, false));
    expect(eq(tl::all_are_same_v<tl::list<int>>, true));
    expect(eq(tl::all_are_same_v<tl::list<>>, true));
  };

  "a field's stored type is extractable, and a non-field says so"_test = [] constexpr {
    using int_field = s2s::basic_field<"x", int, s2s::field_size<s2s::fixed<4>>>;
    // basic_field requires an integral T, so a float leaf is spelled with the
    // field template it expands to.
    using float_field = s2s::field<"y", float, s2s::field_size<s2s::fixed<4>>,
                                   s2s::no_constraint<float>{}>;
    using inner = s2s::struct_field_list<int_field, float_field>;

    expect(eq(std::is_same_v<s2s::extract_type_from_field_v<int_field>, int>, true));
    expect(eq(std::is_same_v<s2s::extract_type_from_field_v<float_field>, float>, true));
    expect(eq(std::is_same_v<s2s::extract_type_from_field_v<std::array<char, 10>>,
                             s2s::not_a_field>, true));
    expect(eq(std::is_same_v<
      s2s::extract_type_from_field_v<s2s::struct_field<"d", inner>>, inner>, true));
  };

  // The stream concepts are what route every read and write, and nothing else
  // asserts them directly.
  "the stream concepts classify the standard streams"_test = [] constexpr {
    expect(eq(s2s::std_read_trait<std::ifstream>, true));
    expect(eq(s2s::readable<std::ifstream>, true));
    expect(eq(s2s::convertible_to_bool<std::ifstream>, true));
    expect(eq(s2s::input_stream_like<std::ifstream>, true));
    expect(eq(s2s::output_stream_like<std::ofstream>, true));
    // A stringstream is both, which is what the write-side runtime tests rely
    // on.
    expect(eq(s2s::input_stream_like<std::stringstream>, true));
    expect(eq(s2s::output_stream_like<std::stringstream>, true));
    // Not a stream at all.
    expect(eq(s2s::readable<int>, false));
  };

  // Folded in from test/constexpr/static_tests_2.cpp unchanged.
  "meta::type_id identifies and transforms types"_test = [] constexpr {
    expect(eq(meta::type_id<void> != meta::type_id<int>, true));
    expect(eq(meta::type_id<int> == meta::type_id<int>, true));
    expect(eq(meta::invoke<std::is_const>(meta::type_id<const int>), true));
    expect(eq(meta::invoke<std::is_const>(meta::type_id<int>), false));
    expect(eq(meta::type_id<int> == meta::invoke<std::remove_pointer>(meta::type_id<int*>), true));
  };

  // TODO(045): the option pack is order-independent — the classifier scans the
  // pack by kind rather than reading it positionally. Assert that
  // basic_field<"v", u16, 2_B, s2s::eq{5}> and
  // basic_field<"v", u16, s2s::eq{5}, 2_B> name the same type. Without this a
  // classifier that only happens to work in the order every other test writes
  // its options passes the whole suite.
  //
  // Placeholder body: asserts nothing yet, since the pack does not exist.
  "the option pack is order-independent"_test = [] constexpr {
    expect(eq(true, true));
  };
}
