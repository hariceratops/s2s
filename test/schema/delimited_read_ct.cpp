// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// Every bound-related case here declares a small max_bytes<79>. The
// delimiter_not_found path at the *default* 16 MiB bound reads one byte at a
// time under constant evaluation, which is well past gcc's
// -fconstexpr-ops-limit and -fconstexpr-loop-limit; that path is exercised in
// delimited_read.cpp (GoogleTest) only.

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
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
using u32 = unsigned int;

using bounded_keyword_then_tail =
  s2s::struct_field_list<
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using bounded_only =
  s2s::struct_field_list<
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>
  >;

// 063: vec_field<u8> shares read_delimited with str_field, instantiated a
// second time — the same five cases, against std::vector<u8>.
using bounded_vector_then_tail =
  s2s::struct_field_list<
    s2s::vec_field<"data", u8, s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using bounded_vector_only =
  s2s::struct_field_list<
    s2s::vec_field<"data", u8, s2s::until<u8{0}>, s2s::max_bytes<79>>
  >;

// 067: a discovered length (until<>) feeding a computed one
// (size_from_fields) — PNG's tEXt chunk. The text runs to the chunk's
// declared end rather than carrying its own delimiter.
constexpr auto remaining = [](auto length, const std::string& keyword) -> std::size_t {
  return length - keyword.size() - 1;
};

using png_text_chunk =
  s2s::struct_field_list<
    s2s::basic_field<"length", u32, 4_B>,
    s2s::str_field<"keyword", s2s::until<u8{0}>, s2s::max_bytes<79>>,
    s2s::str_field<"text", s2s::size_from_fields<remaining, "length", "keyword">>
  >;

auto main() -> int {
  "a NUL-terminated keyword is read and the delimiter is not stored"_test = [] constexpr {
    std::array<u8, 6> buffer{'f', 'o', 'o', 0x00, 0x22, 0x11};
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_keyword_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["keyword"_f].size(), std::size_t{3}));
    expect(eq(std::string_view{(*res)["keyword"_f]}, std::string_view{"foo"}));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  // ELF's string-table index 0: a delimiter in first position is not an
  // error, it is the empty string.
  "a delimiter in first position yields an empty value"_test = [] constexpr {
    std::array<u8, 3> buffer{0x00, 0x22, 0x11};
    memstream<3> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_keyword_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["keyword"_f].empty(), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  // The bound counts the value, not the wire: max_bytes<79> admits a 79-byte
  // value plus its delimiter.
  "max_bytes accepts a value exactly at its declared bound"_test = [] constexpr {
    std::array<u8, 80> buffer{};
    for(std::size_t i = 0; i < 79; ++i)
      buffer[i] = 'a';
    buffer[79] = 0x00;
    memstream<80> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_only>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["keyword"_f].size(), std::size_t{79}));
  };

  // One byte over the bound is rejected: the value's 80th byte is read only
  // to discover it is not the delimiter, and the bound has already been met.
  "max_bytes rejects a value one byte over its declared bound"_test = [] constexpr {
    std::array<u8, 80> buffer{};
    for(std::size_t i = 0; i < 80; ++i)
      buffer[i] = 'a';
    memstream<80> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_only>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::delimiter_not_found));
    expect(eq(res.error().failed_at, std::string_view{"keyword"}));
  };

  // The stream running dry before the delimiter is a different fact than the
  // bound being reached, and reports the leaf's own buffer_exhaustion.
  "a stream that runs dry before the delimiter reports buffer_exhaustion"_test = [] constexpr {
    std::array<u8, 3> buffer{'a', 'b', 'c'};
    memstream<3> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_keyword_then_tail>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"keyword"}));
  };

  // 063: the same matrix against vec_field<u8> — read_delimited instantiated
  // a second time, not duplicated.
  "vec_field reads until the delimiter and drops it"_test = [] constexpr {
    std::array<u8, 6> buffer{'f', 'o', 'o', 0x00, 0x22, 0x11};
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vector_then_tail>(stream);

    constexpr std::array<u8, 3> expected{'f', 'o', 'o'};
    expect(eq(res.has_value(), true));
    expect(eq((*res)["data"_f].size(), std::size_t{3}));
    expect(eq(std::equal((*res)["data"_f].begin(), (*res)["data"_f].end(), expected.begin()), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "vec_field: a delimiter in first position yields an empty value"_test = [] constexpr {
    std::array<u8, 3> buffer{0x00, 0x22, 0x11};
    memstream<3> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vector_then_tail>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["data"_f].empty(), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "vec_field: max_bytes accepts a value exactly at its declared bound"_test = [] constexpr {
    std::array<u8, 80> buffer{};
    for(std::size_t i = 0; i < 79; ++i)
      buffer[i] = 'a';
    buffer[79] = 0x00;
    memstream<80> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vector_only>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["data"_f].size(), std::size_t{79}));
  };

  "vec_field: max_bytes rejects a value one byte over its declared bound"_test = [] constexpr {
    std::array<u8, 80> buffer{};
    for(std::size_t i = 0; i < 80; ++i)
      buffer[i] = 'a';
    memstream<80> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vector_only>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::delimiter_not_found));
    expect(eq(res.error().failed_at, std::string_view{"data"}));
  };

  "vec_field: a stream that runs dry before the delimiter reports buffer_exhaustion"_test = [] constexpr {
    std::array<u8, 3> buffer{'a', 'b', 'c'};
    memstream<3> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_vector_then_tail>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"data"}));
  };

  // 067: a discovered length feeding a computed one. The text's length is
  // the chunk length minus the keyword's discovered length minus the one
  // delimiter byte, not a delimiter of its own.
  "the PNG tEXt shape composes a discovered length with a computed one"_test = [] constexpr {
    std::array<u8, 15> buffer{
      0x0b, 0x00, 0x00, 0x00,
      'f', 'o', 'o', 0x00,
      'b', 'a', 'r', ' ', 'b', 'a', 'z'
    };
    memstream<15> stream(buffer);

    auto res = s2s::struct_cast_le<png_text_chunk>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["keyword"_f]}, std::string_view{"foo"}));
    expect(eq(std::string_view{(*res)["text"_f]}, std::string_view{"bar baz"}));
  };

  // The read compares bytes, not signed chars: a delimiter >= 0x80 must not
  // be missed because char is signed on every platform this library targets.
  "a delimiter at or above 0x80 is matched, not sign-extended past"_test = [] constexpr {
    using high_delim =
      s2s::struct_field_list<
        s2s::str_field<"keyword", s2s::until<u8{0x80}>, s2s::max_bytes<79>>
      >;

    std::array<u8, 4> buffer{'a', 'b', 0x80, 0x11};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<high_delim>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::string_view{(*res)["keyword"_f]}, std::string_view{"ab"}));
  };
}
