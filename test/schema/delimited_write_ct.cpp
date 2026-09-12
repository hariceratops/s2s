// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// 065's happy path plus 066's rejection: a value containing the delimiter,
// anywhere including its last byte, fails at compile time under constant
// evaluation exactly as it does at run time — the check is ordinary code,
// not a diagnostic (design §3.2).

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;

using bounded_keyword_then_tail =
  s2s::struct_field_list<
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using bounded_vector_then_tail =
  s2s::struct_field_list<
    s2s::vec_field<"data", u8, s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

auto main() -> int {
  "the value is emitted followed by one delimiter byte"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_keyword_then_tail obj{};
    obj["keyword"_f] = "foo";
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_keyword_then_tail>(stream, obj);

    expect(eq(written.has_value(), true));
    constexpr std::array<u8, 6> expected{'f', 'o', 'o', 0x00, 0x22, 0x11};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };

  // Empty is a valid value: it emits the lone delimiter, and reads back as
  // an empty value — the round trip 062 made valid on the read side closes
  // here.
  "an empty value emits the lone delimiter and reads back empty"_test = [] constexpr {
    std::array<u8, 3> buffer{};
    memstream<3> stream(buffer);
    bounded_keyword_then_tail obj{};
    obj["keyword"_f] = "";
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_keyword_then_tail>(stream, obj);
    expect(eq(written.has_value(), true));

    stream.rewind();
    auto res = s2s::struct_cast_le<bounded_keyword_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["keyword"_f].empty(), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "a NUL-terminated keyword round trips byte-identically"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_keyword_then_tail obj{};
    obj["keyword"_f] = "foo";
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_keyword_then_tail>(stream, obj);
    expect(eq(written.has_value(), true));

    stream.rewind();
    auto res = s2s::struct_cast_le<bounded_keyword_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["keyword"_f]}, std::string_view{"foo"}));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  // max_bytes<79> admits a 79-byte value plus its delimiter, on the write
  // side too: the bound counts the value, not the wire.
  "a value exactly at its declared bound round trips"_test = [] constexpr {
    std::array<u8, 82> buffer{};
    memstream<82> stream(buffer);
    bounded_keyword_then_tail obj{};
    std::string keyword;
    for(std::size_t i = 0; i < 79; ++i)
      keyword.push_back('a');
    obj["keyword"_f] = keyword;
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_keyword_then_tail>(stream, obj);
    expect(eq(written.has_value(), true));

    stream.rewind();
    auto res = s2s::struct_cast_le<bounded_keyword_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["keyword"_f].size(), std::size_t{79}));
    expect(eq((*res)["keyword"_f], keyword));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  // 063: the same matrix against vec_field<u8> — the same write_field
  // specialization, instantiated a second time.
  "vec_field: the value is emitted followed by one delimiter byte"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_vector_then_tail obj{};
    obj["data"_f] = std::vector<u8>{'f', 'o', 'o'};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_vector_then_tail>(stream, obj);

    expect(eq(written.has_value(), true));
    constexpr std::array<u8, 6> expected{'f', 'o', 'o', 0x00, 0x22, 0x11};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };

  "vec_field: an empty value emits the lone delimiter and reads back empty"_test = [] constexpr {
    std::array<u8, 3> buffer{};
    memstream<3> stream(buffer);
    bounded_vector_then_tail obj{};
    obj["data"_f] = std::vector<u8>{};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_vector_then_tail>(stream, obj);
    expect(eq(written.has_value(), true));

    stream.rewind();
    auto res = s2s::struct_cast_le<bounded_vector_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["data"_f].empty(), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "vec_field: a value round trips until the delimiter"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_vector_then_tail obj{};
    obj["data"_f] = std::vector<u8>{'f', 'o', 'o'};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_vector_then_tail>(stream, obj);
    expect(eq(written.has_value(), true));

    stream.rewind();
    auto res = s2s::struct_cast_le<bounded_vector_then_tail>(stream);

    expect(eq(res.has_value(), true));
    constexpr std::array<u8, 3> expected{'f', 'o', 'o'};
    expect(eq((*res)["data"_f].size(), std::size_t{3}));
    expect(eq(std::equal((*res)["data"_f].begin(), (*res)["data"_f].end(), expected.begin()), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "a value containing the delimiter fails to write"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_keyword_then_tail obj{};
    obj["keyword"_f] = std::string("fo\x00o", 4);
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_keyword_then_tail>(stream, obj);

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::found_delimiter_in_value));
    expect(eq(written.error().failed_at, std::string_view{"keyword"}));
  };

  // §3.3: no "already terminated" exemption. The engine emits the delimiter
  // itself, so a value already ending in one would otherwise emit a second.
  "a value whose last byte is the delimiter is rejected too"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_keyword_then_tail obj{};
    obj["keyword"_f] = std::string("foo\x00", 4);
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_keyword_then_tail>(stream, obj);

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::found_delimiter_in_value));
    expect(eq(written.error().failed_at, std::string_view{"keyword"}));
  };

  "vec_field: a value containing the delimiter fails to write"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_vector_then_tail obj{};
    obj["data"_f] = std::vector<u8>{'f', 'o', 0x00, 'o'};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_vector_then_tail>(stream, obj);

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::found_delimiter_in_value));
    expect(eq(written.error().failed_at, std::string_view{"data"}));
  };

  "vec_field: a value whose last byte is the delimiter is rejected too"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);
    bounded_vector_then_tail obj{};
    obj["data"_f] = std::vector<u8>{'f', 'o', 'o', 0x00};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<bounded_vector_then_tail>(stream, obj);

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::found_delimiter_in_value));
    expect(eq(written.error().failed_at, std::string_view{"data"}));
  };
}
