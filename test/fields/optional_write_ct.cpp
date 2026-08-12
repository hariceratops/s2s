// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

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

using u32 = unsigned int;

auto flag_is_set = [](auto flag) { return flag == 0xdeadbeef; };

using optional_struct =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::basic_field<"payload", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;

constexpr auto make_optional(u32 flag, bool engaged) -> optional_struct {
  optional_struct obj{};
  obj["flag"_f] = flag;
  if(engaged)
    obj["payload"_f] = 0xcafed00d;
  return obj;
}

constexpr auto write_present_but_empty() -> s2s::cast_result {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  return s2s::stream_cast_le<optional_struct>(stream, make_optional(0xdeadbeef, false));
}

constexpr auto write_absent_but_engaged() -> s2s::cast_result {
  std::array<u8, 8> buffer{};
  memstream<8> stream(buffer);
  return s2s::stream_cast_le<optional_struct>(stream, make_optional(0x11223344, true));
}

// "len" sizes a vector inside a maybe, so it is a *conditional* source: the
// optional may be absent, and then there is nothing to derive from. It stays
// writable and is verified only when the optional is present.
using conditional_len_struct =
  s2s::struct_field_list<
    s2s::basic_field<"flag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"len", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::vec_field<"data", u8, s2s::field_size<s2s::len_from_field<"len">>>,
      s2s::parse_if<flag_is_set, s2s::with_fields<"flag">>
    >
  >;

constexpr auto make_conditional(u32 stored_len, bool engaged) -> conditional_len_struct {
  conditional_len_struct obj{};
  obj["flag"_f] = engaged ? 0xdeadbeef : 0x11223344;
  obj["len"_f] = stored_len;
  if(engaged)
    obj["data"_f] = std::vector<u8>{0xaa, 0xbb, 0xcc};
  return obj;
}

constexpr auto write_disagreeing_conditional_len() -> s2s::cast_result {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  return s2s::stream_cast_be<conditional_len_struct>(stream, make_conditional(5, true));
}

constexpr auto write_unverified_conditional_len() -> s2s::cast_result {
  std::array<u8, 11> buffer{};
  memstream<11> stream(buffer);
  return s2s::stream_cast_be<conditional_len_struct>(stream, make_conditional(99, false));
}

int main() {
  "little endian optionals round trip when present"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    expect(eq(s2s::stream_cast_le<optional_struct>(
      stream, make_optional(0xdeadbeef, true)).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<optional_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["payload"_f].has_value(), true));
    expect(eq(*(*res)["payload"_f], 0xcafed00du));
  };

  "big endian optionals round trip when present"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    expect(eq(s2s::stream_cast_be<optional_struct>(
      stream, make_optional(0xdeadbeef, true)).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<optional_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["payload"_f].has_value(), true));
    expect(eq(*(*res)["payload"_f], 0xcafed00du));
  };

  // An absent optional contributes no bytes at all, so the flag is the whole
  // stream.
  "an absent optional emits nothing"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    expect(eq(s2s::stream_cast_be<optional_struct>(
      stream, make_optional(0x11223344, false)).has_value(), true));
    constexpr std::array<u8, 8> expected{0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));

    stream.rewind();
    auto res = s2s::struct_cast_be<optional_struct>(stream);
    expect(eq(res.has_value(), true));
    expect(eq((*res)["payload"_f].has_value(), false));
  };

  // The reader would go looking for four bytes that were never written.
  "a satisfied predicate with an empty optional is rejected"_test = [] constexpr {
    auto written = write_present_but_empty();

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failed_at, std::string_view{"payload"}));
  };

  // Emitting nothing here would silently discard data the caller handed in.
  "an unsatisfied predicate with an engaged optional is rejected"_test = [] constexpr {
    auto written = write_absent_but_engaged();

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failed_at, std::string_view{"payload"}));
  };

  "a conditional length round trips when the optional is present"_test = [] constexpr {
    std::array<u8, 11> buffer{};
    memstream<11> stream(buffer);

    expect(eq(s2s::stream_cast_be<conditional_len_struct>(
      stream, make_conditional(3, true)).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<conditional_len_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["len"_f], 3u));
    expect(eq((*res)["data"_f].has_value(), true));
    expect(eq((*res)["data"_f]->size(), std::size_t{3}));
  };

  // A conditional source is not derived, so a wrong length is the caller's to
  // get wrong — and must be caught rather than written out.
  "a conditional length that disagrees when present is rejected"_test = [] constexpr {
    auto written = write_disagreeing_conditional_len();

    expect(eq(written.has_value(), false));
    expect(eq(written.error().failure_reason, s2s::error_reason::found_contradicting_length));
    expect(eq(written.error().failed_at, std::string_view{"len"}));
  };

  // When the optional is absent there is no obligation at all, so the length
  // keeps whatever the caller stored.
  "a conditional length is left alone when the optional is absent"_test = [] constexpr {
    expect(eq(write_unverified_conditional_len().has_value(), true));
  };
}
