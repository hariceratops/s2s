// Backs the four-axes example in UserGuide.md's Schema section. The region
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
    s2s::basic_field<"device_id", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"revision", u8, s2s::field_size<s2s::fixed<1>>, s2s::any_of{u8{1}, u8{2}}>,
    s2s::basic_field<"sample_count", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"samples", u16, s2s::field_size<s2s::len_from_field<"sample_count">>>
  >;

auto main() -> int {
  telemetry_frame frame{};
  frame["marker"_f] = std::array<u8, 2>{0xab, 0xcd};
  frame["device_id"_f] = u16{0x2a};
  frame["revision"_f] = u8{2};
  frame["samples"_f] = std::vector<u16>{300, 301, 299, 302};
  // "sample_count" is not assigned: it is the size axis of "samples" resolved.

  std::fstream file("telemetry_frame.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if(!file)
    return 1;

  if(const auto written = s2s::stream_cast_be<telemetry_frame>(file, frame); !written)
    return 1;

  // A file stream shares one position between reads and writes, so rewind
  // before parsing the bytes just emitted.
  file.seekg(0);

  const auto parsed = s2s::struct_cast_be<telemetry_frame>(file);
  if(!parsed)
    return 1;

  return (*parsed)["sample_count"_f] == 4
      && (*parsed)["device_id"_f] == 0x2a
      && (*parsed)["samples"_f][2] == 299 ? 0 : 1;
}
// docs-end
