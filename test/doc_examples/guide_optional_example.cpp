// Backs the maybe/presence-deduction example in docs/schema/optional-and-variant.md. The region
// between docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// Bit 3 of the flags byte says an original-filename length follows. The
// predicate reads a field already parsed, which is why it must name it.
constexpr auto has_name = [](auto flags) { return (flags & 0x08u) != 0u; };

// A gzip member header, trimmed to the part that shows conditional presence.
using gzip_header =
  s2s::struct_field_list<
    s2s::magic_byte_array<"magic", 2, std::array<u8, 2>{0x1f, 0x8b}>,
    s2s::basic_field<"method", u8, 1_B>,
    s2s::basic_field<"flags", u8, 1_B>,
    s2s::basic_field<"mtime", u32, 4_B>,
    s2s::maybe<
      s2s::basic_field<"name_length", u16, 2_B>,
      s2s::parse_if<has_name, "flags">
    >
  >;

auto round_trip(u8 flags, bool with_name, const char* path) -> bool {
  gzip_header header{};
  header["magic"_f] = std::array<u8, 2>{0x1f, 0x8b};
  header["method"_f] = u8{8};
  header["flags"_f] = flags;
  header["mtime"_f] = 0x5f000000u;
  if(with_name)
    header["name_length"_f] = u16{12};

  std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  return s2s::stream_cast_be<gzip_header>(file, header)
    .and_then([&file] {
      file.seekg(0);
      return s2s::struct_cast_be<gzip_header>(file);
    })
    .transform([with_name](const gzip_header& parsed) {
      // An absent optional is empty; a present one is dereferenced.
      return with_name ? *(parsed["name_length"_f]) == 12 : true;
    })
    .value_or(false);
}

auto main() -> int {
  // FNAME set: the predicate says present, and the header supplies it.
  if(!round_trip(u8{0x08}, true, "gzip_named.bin"))
    return 1;

  // FNAME clear: the predicate says absent, and the field occupies no bytes.
  if(!round_trip(u8{0x00}, false, "gzip_plain.bin"))
    return 1;

  // Disagreement is caught: the flag promises a name length, the header has none.
  gzip_header bad{};
  bad["magic"_f] = std::array<u8, 2>{0x1f, 0x8b};
  bad["flags"_f] = u8{0x08};
  std::fstream discard("gzip_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto rejected = s2s::stream_cast_be<gzip_header>(discard, bad);
  if(rejected.has_value())
    return 1;

  return rejected.error().failure_reason == s2s::error_reason::validation_failure ? 0 : 1;
}
// docs-end
