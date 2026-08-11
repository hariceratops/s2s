// Backs the constraints example in UserGuide.md. The region between docs-begin
// and docs-end is compared against that fenced block by the doc_examples_match
// test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <fstream>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// The fmt chunk of a WAV file. Most of what makes a fmt chunk valid is a
// constraint on a single field, so most of it is declared rather than checked.
using wav_format =
  s2s::struct_field_list<
    s2s::magic_byte_array<"chunk_id", 4, std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20}>,
    s2s::basic_field<"chunk_size", u32, s2s::field_size<s2s::fixed<4>>, s2s::eq{u32{16}}>,
    s2s::basic_field<"audio_format", u16, s2s::field_size<s2s::fixed<2>>, s2s::eq{u16{1}}>,
    s2s::basic_field<"channels", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{1}, u16{2}}>,
    s2s::basic_field<"sample_rate", u32, s2s::field_size<s2s::fixed<4>>, s2s::gte{u32{8000}}>,
    s2s::basic_field<"bits_per_sample", u16, s2s::field_size<s2s::fixed<2>>, s2s::any_of{u16{8}, u16{16}, u16{24}}>
  >;

// The same layout with the constraint axis left off every field.
using wav_format_unchecked =
  s2s::struct_field_list<
    s2s::magic_byte_array<"chunk_id", 4, std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20}>,
    s2s::basic_field<"chunk_size", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"audio_format", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"channels", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"sample_rate", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"bits_per_sample", u16, s2s::field_size<s2s::fixed<2>>>
  >;

auto main() -> int {
  wav_format fmt{};
  fmt["chunk_id"_f] = std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20};
  fmt["chunk_size"_f] = 16u;
  fmt["audio_format"_f] = u16{1};
  fmt["channels"_f] = u16{2};
  fmt["sample_rate"_f] = 44100u;
  fmt["bits_per_sample"_f] = u16{16};

  std::fstream file("wav_format.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto accepted =
    s2s::stream_cast_be<wav_format>(file, fmt)
      .and_then([&file] {
        file.seekg(0);
        return s2s::struct_cast_be<wav_format>(file);
      })
      .transform([](const wav_format&) { return true; });

  if(!accepted.value_or(false))
    return 1;

  // Writing: the constraint is checked before the field's first byte is out.
  wav_format surround = fmt;
  surround["channels"_f] = u16{6};
  std::fstream discard("wav_format_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto refused = s2s::stream_cast_be<wav_format>(discard, surround);
  if(refused.has_value()
     || refused.error().failure_reason != s2s::error_reason::validation_failure
     || refused.error().failed_at != std::string_view{"channels"})
    return 1;

  // Reading: the same constraint rejects the same value coming off the wire.
  // The bytes have to exist first, so they are written through the unchecked
  // layout and parsed back through the checked one.
  wav_format_unchecked loose{};
  loose["chunk_id"_f] = std::array<u8, 4>{0x66, 0x6d, 0x74, 0x20};
  loose["chunk_size"_f] = 16u;
  loose["audio_format"_f] = u16{1};
  loose["channels"_f] = u16{6};
  loose["sample_rate"_f] = 44100u;
  loose["bits_per_sample"_f] = u16{16};

  std::fstream on_disk("wav_format_loose.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  const auto parsed =
    s2s::stream_cast_be<wav_format_unchecked>(on_disk, loose)
      .and_then([&on_disk] {
        on_disk.seekg(0);
        return s2s::struct_cast_be<wav_format>(on_disk);
      });

  if(parsed.has_value())
    return 1;

  return parsed.error().failure_reason == s2s::error_reason::validation_failure
      && parsed.error().failed_at == std::string_view{"channels"} ? 0 : 1;
}
// docs-end
