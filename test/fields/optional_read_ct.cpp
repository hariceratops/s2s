// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// The predicate lambdas are namespace scope on purpose. They are NTTPs of the
// schema, not captures of the test body.

#include <array>
#include <cstddef>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u32 = unsigned int;

auto flag_is_set = [](auto flag) { return flag == 0xdeadbeef; };

using optional_trivial =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;

using optional_array =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::fixed_array_field<"c", u32, 3>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"a">>
    >
  >;

using point =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;

using optional_record =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<s2s::struct_field<"inner", point>,
               s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>>
  >;

using unconditional =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::always_present
    >
  >;

int main() {
  "a satisfied predicate makes the payload present"_test = [] constexpr {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<optional_trivial>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["payload"_f].has_value(), true));
    expect(eq(*(*res)["payload"_f], 0xcafed00du));
  };

  // The four payload bytes are on the wire either way. An unsatisfied
  // predicate must leave them unread rather than read and discard them.
  "an unsatisfied predicate leaves the payload absent and unconsumed"_test = [] constexpr {
    using trailing =
      s2s::struct_field_list<
        s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::maybe<
          s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
          s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
        >,
        s2s::basic_field<"tail", u32, s2s::field_size<s2s::fixed<4>>>
      >;

    std::array<u8, 8> buffer{0x44, 0x33, 0x22, 0x11, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<trailing>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["payload"_f].has_value(), false));
    // "tail" gets the bytes the absent payload did not take.
    expect(eq((*res)["tail"_f], 0xcafed00du));
  };

  "an optional fixed array is read whole when present"_test = [] constexpr {
    std::array<u8, 20> buffer{
      0xef, 0xbe, 0xad, 0xde,
      0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde,
      0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xef, 0xbe
    };
    memstream<20> stream(buffer);

    auto res = s2s::struct_cast_le<optional_array>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
    expect(eq((*res)["c"_f].has_value(), true));
    expect(eq((*(*res)["c"_f])[0], 0xdeadbeefu));
    expect(eq((*(*res)["c"_f])[1], 0xcafed00du));
    expect(eq((*(*res)["c"_f])[2], 0xbeefbeefu));
  };

  // A wrapped record has to recurse into its own leaves, which a trivial
  // payload never exercises.
  "an optional record is read when present"_test = [] constexpr {
    std::array<u8, 12> buffer{
      0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xef, 0xbe, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<optional_record>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["inner"_f].has_value(), true));
    expect(eq((*(*res)["inner"_f])["x"_f], 0xbeefbeefu));
    expect(eq((*(*res)["inner"_f])["y"_f], 0xcafed00du));
  };

  "the payload takes the declared byte order when present"_test = [] constexpr {
    std::array<u8, 8> buffer{0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xd0, 0x0d};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_be<optional_trivial>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["payload"_f].has_value(), true));
    expect(eq(*(*res)["payload"_f], 0xcafed00du));
  };

  // always_present is a predicate over no fields at all, so it has to hold
  // whatever the siblings say.
  "always_present makes the payload present regardless of siblings"_test = [] constexpr {
    std::array<u8, 8> buffer{0x44, 0x33, 0x22, 0x11, 0x0d, 0xd0, 0xfe, 0xca};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<unconditional>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["flag"_f], 0x11223344u));
    expect(eq((*res)["payload"_f].has_value(), true));
    expect(eq(*(*res)["payload"_f], 0xcafed00du));
  };

  // Presence does not excuse a short buffer: the predicate holds, so the
  // bytes are owed.
  "a present payload with no bytes left fails the read"_test = [] constexpr {
    std::array<u8, 6> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0};
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast_le<optional_trivial>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"payload"}));
  };
}
