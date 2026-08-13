// Backs the worked example in docs/writing.md. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
//
// doc_example.cpp is the other half of this pair: it verifies the prose claims
// the Writing section makes about derived fields, which are asserted rather
// than shown and so are not part of any fenced block.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <string>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;

// A length-prefixed log record. The length is not data anyone supplies; it is
// a consequence of the message.
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
  // "message_length" is never assigned. It is derived from message.size().

  std::fstream file("log_record.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto round_tripped =
    s2s::stream_cast_be<log_record>(file, record)
      .and_then([&file] {
        // A file stream shares one position between reads and writes.
        file.seekg(0);
        return s2s::struct_cast_be<log_record>(file);
      })
      .transform([](const log_record& parsed) {
        return parsed["message"_f].size() == 16
            && parsed["message"_f] == "disk nearly full";
      });

  return round_tripped.value_or(false) ? 0 : 1;
}
// docs-end
