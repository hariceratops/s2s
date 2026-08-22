// Backs the four-axes example in docs/schema/index.md. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// A telemetry frame: a two-byte marker, the device that sent it, a protocol
// revision, and a run of samples whose count is carried on the wire.
using telemetry_frame =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0xab, 0xcd}>,
    s2s::basic_field<"device_id", u16, 2_B>,
    s2s::basic_field<"revision", u8, 1_B, s2s::any_of{u8{1}, u8{2}}>,
    s2s::basic_field<"sample_count", u32, 4_B>,
    s2s::vec_field<"samples", u16, s2s::len_from_field<"sample_count">>
  >;

auto main() -> int {
  telemetry_frame frame{};
  frame["device_id"_f] = u16{0x2a};
  frame["revision"_f] = u8{2};
  frame["samples"_f] = std::vector<u16>{300, 301, 299, 302};
  // "sample_count" is not assigned: it is the size axis of "samples" resolved.

  std::fstream file("telemetry_frame.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  // Every step yields an expected, so the steps chain instead of nesting. A
  // failure anywhere short-circuits and carries its cast_error to the end.
  const auto round_tripped =
    s2s::stream_cast_be<telemetry_frame>(file, frame)
      .and_then([&file] {
        // A file stream shares one position between reads and writes.
        file.seekg(0);
        return s2s::struct_cast_be<telemetry_frame>(file);
      })
      .transform([](const telemetry_frame& parsed) {
        return parsed["samples"_f].size() == 4
            && parsed["device_id"_f] == 0x2a
            && parsed["samples"_f][2] == 299;
      });

  return round_tripped.value_or(false) ? 0 : 1;
}
// docs-end
