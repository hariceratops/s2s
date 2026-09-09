// Backs the worked example in docs/schema/byte-order-axis.md. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <bit>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// A TIFF file opens with two bytes naming the byte order the rest of it uses:
// "II" for little-endian, "MM" for big. Everything after them — starting with
// the 42 that confirms the format — is read in whichever order they name.
using byte_order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using tiff_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", byte_order_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"first_ifd_offset", u32, 4_B>
  >;

// The same header in both orders. Only the first two bytes say so; the four
// after them are the same number written two ways.
constexpr unsigned char little_endian_tiff[] = {
  'I', 'I', 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00
};

constexpr unsigned char big_endian_tiff[] = {
  'M', 'M', 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
};

auto write_sample(const char* path, const unsigned char (&bytes)[8]) -> bool {
  std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(bytes), 8);
  return static_cast<bool>(out);
}

auto first_ifd_offset(const char* path) -> std::expected<u32, s2s::cast_error> {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  return s2s::struct_cast<tiff_header>(file)
    .transform([](const tiff_header& header) { return header["first_ifd_offset"_f]; });
}

auto main() -> int {
  if(!write_sample("little.tiff", little_endian_tiff))
    return 1;
  if(!write_sample("big.tiff", big_endian_tiff))
    return 1;

  // One schema, one call, no byte order named anywhere: the files decide, and
  // both yield 8.
  if(first_ifd_offset("little.tiff").value_or(0) != 8)
    return 1;
  if(first_ifd_offset("big.tiff").value_or(0) != 8)
    return 1;

  // On the way out the marker held in the struct decides the order, so setting
  // it is how a caller chooses what the file will look like.
  tiff_header header{};
  header["byte_order"_f]["marker"_f] = std::array<u8, 2>{'M', 'M'};
  header["first_ifd_offset"_f] = 8;

  std::ofstream out("written.tiff", std::ios::out | std::ios::binary | std::ios::trunc);
  if(!s2s::stream_cast<tiff_header>(out, header))
    return 1;
  out.close();

  // A file whose marker is neither II nor MM is not a TIFF, and is rejected the
  // way a bad magic number is: validation_failure, at the announcing field.
  constexpr unsigned char not_a_tiff[] = {
    'X', 'X', 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00
  };
  if(!write_sample("other.bin", not_a_tiff))
    return 1;

  const auto rejected = first_ifd_offset("other.bin");
  if(rejected.has_value())
    return 1;

  return rejected.error().failure_reason == s2s::error_reason::validation_failure
      && rejected.error().failed_at == std::string_view{"byte_order"} ? 0 : 1;
}
// docs-end
