// Backs the write-direction example in README.md. The region between the
// docs-begin and docs-end markers is compared against that fenced block by the
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

// One schema, both directions.
using firmware_image =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x46, 0x57}>,
    s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"payload_length", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"payload", u8, s2s::field_size<s2s::len_from_field<"payload_length">>>
  >;

auto main() -> int {
  firmware_image image{};
  image["marker"_f] = std::array<u8, 2>{0x46, 0x57};
  image["version"_f] = u16{1};
  image["payload"_f] = std::vector<u8>{0xde, 0xad, 0xbe, 0xef};
  // "payload_length" is never assigned. It is derived from payload.size().

  std::fstream file("firmware_out.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if(!file)
    return 1;

  if(const auto written = s2s::struct_write_be<firmware_image>(file, image); !written)
    return 1;

  // A file stream shares one position between reads and writes, so rewind
  // before parsing the bytes just emitted.
  file.seekg(0);
  const auto parsed = s2s::struct_cast_be<firmware_image>(file);

  return parsed && (*parsed)["payload_length"_f] == 4 ? 0 : 1;
}
// docs-end
