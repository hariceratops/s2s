// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u32 = unsigned int;

using point =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;

auto main() -> int {
  "a nested schema reads its own leaves"_test = [] constexpr {
    using nested =
      s2s::struct_field_list<
        s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::struct_field<"c", point>
      >;

    std::array<u8, 16> buffer{
      0xef, 0xbe, 0xad, 0xde,
      0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xef, 0xbe,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<nested>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["b"_f], 0xcafed00du));
    expect(eq((*res)["c"_f]["x"_f], 0xbeefbeefu));
    expect(eq((*res)["c"_f]["y"_f], 0xcafed00du));
  };

  // Endianness is applied per leaf, not per record, so the inner fields have
  // to come back reversed rather than the record's bytes as a block.
  "a nested schema takes the declared byte order at every leaf"_test = [] constexpr {
    using nested =
      s2s::struct_field_list<
        s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::struct_field<"c", point>
      >;

    std::array<u8, 12> buffer{
      0xde, 0xad, 0xbe, 0xef,
      0xbe, 0xef, 0xbe, 0xef,
      0xca, 0xfe, 0xd0, 0x0d
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_be<nested>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0xdeadbeefu));
    expect(eq((*res)["c"_f]["x"_f], 0xbeefbeefu));
    expect(eq((*res)["c"_f]["y"_f], 0xcafed00du));
  };

  "an array of records reads every element"_test = [] constexpr {
    using schema =
      s2s::struct_field_list<
        s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
        s2s::array_of_records<"records", point, 3>
      >;

    std::array<u8, 32> buffer{
      0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<32> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], std::size_t{3}));
    for(std::size_t idx = 0; idx < 3; ++idx) {
      expect(eq((*res)["records"_f][idx]["x"_f], 0xdeadbeefu));
      expect(eq((*res)["records"_f][idx]["y"_f], 0xcafed00du));
    }
  };

  "an array of records takes the declared byte order at every leaf"_test = [] constexpr {
    using schema = s2s::struct_field_list<s2s::array_of_records<"records", point, 2>>;

    std::array<u8, 16> buffer{
      0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xd0, 0x0d,
      0xbe, 0xef, 0xbe, 0xef, 0x11, 0x22, 0x33, 0x44
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_be<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["records"_f][0]["x"_f], 0xdeadbeefu));
    expect(eq((*res)["records"_f][1]["y"_f], 0x11223344u));
  };

  // The old constexpr suite has this case commented out as impossible. It is
  // not: what leaks is a namespace-scope `constexpr auto` holding the vector,
  // and a test body holds nothing past constant evaluation.
  "a vector of records takes its element count from the length field"_test = [] constexpr {
    using schema =
      s2s::struct_field_list<
        s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
        s2s::vector_of_records<"records", point, s2s::field_size<s2s::len_from_field<"len">>>
      >;

    std::array<u8, 32> buffer{
      0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<32> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], std::size_t{3}));
    expect(eq((*res)["records"_f].size(), std::size_t{3}));
    for(std::size_t idx = 0; idx < 3; ++idx) {
      expect(eq((*res)["records"_f][idx]["x"_f], 0xdeadbeefu));
      expect(eq((*res)["records"_f][idx]["y"_f], 0xcafed00du));
    }
  };

  // A record boundary is not a resynchronisation point: running out inside an
  // element fails the whole read rather than returning the elements before it.
  "a buffer that ends inside a record element fails the read"_test = [] constexpr {
    using schema = s2s::struct_field_list<s2s::array_of_records<"records", point, 2>>;

    std::array<u8, 12> buffer{
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca, 0xef, 0xbe, 0xad, 0xde};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
  };
}
