// The derived-field claims UserGuide.md's Writing section makes in prose,
// asserted rather than shown. They are not part of any fenced block, so the
// doc_examples_match test cannot cover them; this target does.
//
// guide_writing_example.cpp is the other half of the pair, and carries the
// worked example that section displays.
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <string>
#include <utility>
#include <cstdio>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;

using log_record =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x4c, 0x47}>,
    s2s::basic_field<"message_length", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::str_field<"message", s2s::field_size<s2s::len_from_field<"message_length">>>
  >;

auto main() -> int {
  log_record record{};
  record["marker"_f] = std::array<u8, 2>{0x4c, 0x47};
  record["message"_f] = std::string("disk nearly full");

  // The guide states that reading a derived field gives the stored slot, not
  // the derived value, and that nothing is written back during a write.
  if(record["message_length"_f] != 0) {
    std::printf("a derived slot is populated before any write\n");
    return 1;
  }
  const auto through_const = std::as_const(record)["message_length"_f];
  if(through_const != 0)
    return 1;

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

  if(!parsed.has_value() || (*parsed)["message_length"_f] != 16)
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
      .transform([](const log_record& r) { return r["message_length"_f] == 2; });

  if(!reread.value_or(false)) {
    std::printf("assigning the container did not move the derived length\n");
    return 1;
  }

  std::printf("doc claims ok\n");
  return 0;
}
