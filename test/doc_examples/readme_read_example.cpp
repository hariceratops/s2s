// Backs the read-direction example in README.md. The region between docs-begin
// and docs-end is compared against that fenced block by the doc_examples_match
// test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <print>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// A firmware image: a two-byte marker, a version, and a payload whose length
// is carried on the wire just before it.
using firmware_image =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x46, 0x57}>,
    s2s::basic_field<"version", u16, s2s::field_size<s2s::fixed<2>>>,
    s2s::basic_field<"payload_length", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"payload", u8, s2s::field_size<s2s::len_from_field<"payload_length">>>
  >;

auto main() -> int {
  std::ifstream image("firmware.bin", std::ios::in | std::ios::binary);

  const auto res =
    s2s::struct_cast_be<firmware_image>(image)
      .transform([](const firmware_image& fields){
        std::println("version={} payload={} bytes",
                     fields["version"_f], fields["payload"_f].size());
        return fields;
      }).transform_error([](const s2s::cast_error& err){
        std::println("failure_reason={} failed_at={}",
                     static_cast<int>(err.failure_reason), err.failed_at);
        return err;
      });

  return res.has_value() ? 0 : 1;
}
// docs-end
