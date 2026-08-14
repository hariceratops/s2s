// Rewrite of test/constexpr/static_tests_1.cpp, which was unregistered in
// CMake and written against an API several generations old. Every assertion
// here was checked against the current headers; see 038's notes for what was
// dropped and why.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <cstdint>
#include <fstream>
#include <limits>
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

using u16 = unsigned short;
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
    expect(eq(s2s::is_fixed_size_v<s2s::size_type_of<4_B>>, true));
    expect(eq(s2s::is_fixed_size_v<int>, false));
    expect(eq(s2s::is_variable_size_v<int>, false));
    // The width lives in the value, and deduce_field_size is what reads it.
    expect(eq(s2s::deduce_field_size<6_B>{}(), std::size_t{6}));
    expect(eq(s2s::deduce_field_size<2_B>{}(), std::size_t{2}));
  };

  "an optional field is distinguishable from a plain one"_test = [] constexpr {
    using optional_f =
      s2s::maybe<
        s2s::basic_field<"a", u32, 4_B>,
        s2s::parse_if<is_eq_1, "a">
      >;
    using plain_f = s2s::basic_field<"a", u32, 4_B>;

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
    using int_field = s2s::basic_field<"x", int, 4_B>;
    // basic_field requires an integral T, so a float leaf is spelled with the
    // field template it expands to.
    using float_field = s2s::field<"y", float, 4_B,
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

  // checked_byte_count is phrased as a division so the product is never
  // evaluated on an unvalidated length. The ceiling is spelled out rather than
  // defaulted: once 047 lowers the default, a wrapping length is rejected by
  // the bound long before it can wrap, and a test leaning on the default would
  // quietly stop covering overflow at all.
  "a byte count that would overflow is rejected"_test = [] constexpr {
    constexpr auto ceiling = std::numeric_limits<std::size_t>::max();
    constexpr auto widest = ceiling / sizeof(std::uint64_t);

    // Exactly at the last representable count: accepted, and the product is
    // the real one.
    constexpr auto ok = s2s::checked_byte_count<std::uint64_t, ceiling>(widest);
    expect(eq(ok.has_value(), true));
    expect(eq(*ok, widest * sizeof(std::uint64_t)));

    // One more would wrap.
    constexpr auto wrapped = s2s::checked_byte_count<std::uint64_t, ceiling>(widest + 1);
    expect(eq(wrapped.has_value(), false));
    expect(eq(wrapped.error(), s2s::error_reason::excessive_length));

    // A byte-sized element cannot overflow at all, so nothing is rejected.
    constexpr auto bytes = s2s::checked_byte_count<unsigned char, ceiling>(ceiling);
    expect(eq(bytes.has_value(), true));
    expect(eq(*bytes, ceiling));
  };

  // A ceiling below SIZE_MAX rejects on the bound rather than on the wrap, by
  // the same comparison — this is what 047 turns on.
  "the same gate rejects on a lowered ceiling"_test = [] constexpr {
    constexpr auto at = s2s::checked_byte_count<std::uint32_t, 8>(2);
    expect(eq(at.has_value(), true));
    expect(eq(*at, std::size_t{8}));

    constexpr auto over = s2s::checked_byte_count<std::uint32_t, 8>(3);
    expect(eq(over.has_value(), false));

    // A ceiling that is not a whole number of elements truncates down.
    constexpr auto truncated = s2s::checked_byte_count<std::uint32_t, 10>(2);
    expect(eq(truncated.has_value(), true));
    expect(eq(s2s::checked_byte_count<std::uint32_t, 10>(3).has_value(), false));
  };

  // The classifier scans the pack by kind rather than reading it positionally.
  // Without this a classifier that only happened to work in the order every
  // other test writes its options would pass the whole suite.
  "the option pack is order-independent"_test = [] constexpr {
    using size_then_constraint = s2s::basic_field<"v", u16, 2_B, s2s::eq{u16{5}}>;
    using constraint_then_size = s2s::basic_field<"v", u16, s2s::eq{u16{5}}, 2_B>;
    expect(eq(std::is_same_v<size_then_constraint, constraint_then_size>, true));

    // Either option may be omitted, and the defaults do not depend on which.
    using size_only = s2s::basic_field<"v", u16, 2_B>;
    using constraint_only = s2s::basic_field<"v", u16, s2s::eq{u16{5}}>;
    using neither = s2s::basic_field<"v", u16>;
    expect(eq(std::is_same_v<size_only, neither>, true));
    expect(eq(std::is_same_v<constraint_only, constraint_then_size>, true));
  };
}
