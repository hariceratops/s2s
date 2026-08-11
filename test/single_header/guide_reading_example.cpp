// Backs the worked example in UserGuide.md's Reading section. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

// The 14-byte header every BMP file starts with. BMP is a little-endian
// format, so it is read with struct_cast_le.
using bmp_header =
  s2s::struct_field_list<
    s2s::magic_byte_array<"signature", 2, std::array<u8, 2>{0x42, 0x4d}>,
    s2s::basic_field<"file_size", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"reserved", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"pixel_offset", u32, s2s::field_size<s2s::fixed<4>>>
  >;

// 'B' 'M', then file_size, reserved and pixel_offset, each little-endian.
constexpr unsigned char header_bytes[] = {
  0x42, 0x4d,
  0x36, 0x00, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x36, 0x00, 0x00, 0x00
};

auto write_sample(const char* path, std::size_t count) -> bool {
  std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(header_bytes),
            static_cast<std::streamsize>(count));
  return static_cast<bool>(out);
}

auto main() -> int {
  if(!write_sample("sample.bmp", sizeof(header_bytes)))
    return 1;

  std::ifstream file("sample.bmp", std::ios::in | std::ios::binary);

  const auto parsed =
    s2s::struct_cast_le<bmp_header>(file)
      .transform([](const bmp_header& header) {
        return header["file_size"_f] == 0x000c0036 && header["pixel_offset"_f] == 54;
      });

  if(!parsed.value_or(false))
    return 1;

  // A file that stops short reports which field ran out, not merely that one did.
  if(!write_sample("truncated.bmp", 8))
    return 1;

  std::ifstream short_file("truncated.bmp", std::ios::in | std::ios::binary);
  const auto failed = s2s::struct_cast_le<bmp_header>(short_file);
  if(failed.has_value())
    return 1;

  return failed.error().failure_reason == s2s::error_reason::buffer_exhaustion
      && failed.error().failed_at == std::string_view{"reserved"} ? 0 : 1;
}
// docs-end
