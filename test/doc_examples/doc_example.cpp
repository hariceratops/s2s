// The derived-field claims docs/writing.md makes in prose,
// asserted rather than shown. They are not part of any fenced block, so the
// doc_examples_match test cannot cover them; this target does.
//
// guide_writing_example.cpp is the other half of the pair, and carries the
// worked example that section displays.
//
// The claims that a length target cannot be named at all, on either subscript,
// are the one part this file cannot make: a program asserting them would not
// compile. test/must_not_compile/hidden_length_target.cpp carries those.
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <string>
#include <utility>
#include <variant>
#include <cstdio>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using log_record =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x4c, 0x47}>,
    s2s::basic_field<"message_length", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::str_field<"message", s2s::field_size<s2s::len_from_field<"message_length">>>
  >;

using alt_1 = s2s::struct_field_list<s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>>;
using alt_2 = s2s::struct_field_list<s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>>;

using tagged =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<"body", s2s::type<
      s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
        s2s::match_case<0xdeadbeef, s2s::as_struct<alt_2>>
      >
    >>
  >;

auto main() -> int {
  log_record record{};
  record["marker"_f] = std::array<u8, 2>{0x4c, 0x47};
  record["message"_f] = std::string("disk nearly full");

  // The guide states the container is where to ask for a length, since the
  // length target itself is no longer nameable.
  if(record["message"_f].size() != 16) {
    std::printf("the container does not report its own length\n");
    return 1;
  }

  // The guide states a discriminant is still readable through either
  // subscript, and gives the stored slot rather than the derived value.
  tagged tag_record{};
  tag_record["body"_f] = alt_1{};
  if(tag_record["tag"_f] != 0 || std::as_const(tag_record)["tag"_f] != 0) {
    std::printf("a discriminant slot is populated before any write\n");
    return 1;
  }

  std::fstream le("doc_claims_le.bin",
                  std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if(!s2s::stream_cast_le<log_record>(le, record).has_value())
    return 1;

  std::fstream be("doc_claims_be.bin",
                  std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto parsed =
    s2s::stream_cast_be<log_record>(be, record)
      .and_then([&be] {
        be.seekg(0);
        return s2s::struct_cast_be<log_record>(be);
      });

  // The length went onto the wire and sized the container coming back, which
  // is the whole of what a hidden target still does.
  if(!parsed.has_value() || (*parsed)["message"_f].size() != 16)
    return 1;

  // The guide's replacement for the no-longer-compiling assignment to a
  // derived length: assign the container, and the length follows.
  auto reopened = *parsed;
  reopened["message"_f] = std::string("ok");

  std::fstream again("doc_claims_again.bin",
                     std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto reread =
    s2s::stream_cast_be<log_record>(again, reopened)
      .and_then([&again] {
        again.seekg(0);
        return s2s::struct_cast_be<log_record>(again);
      })
      .transform([](const log_record& r) { return r["message"_f].size() == 2; });

  if(!reread.value_or(false)) {
    std::printf("assigning the container did not move the derived length\n");
    return 1;
  }

  std::printf("doc claims ok\n");
  return 0;
}
