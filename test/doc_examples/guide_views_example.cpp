// Backs the worked example in docs/views.md. The region between docs-begin
// and docs-end is compared against that fenced block by the doc_examples_match
// test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <fstream>
#include <iterator>
#include <span>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

// The 24-byte header of a classic pcap capture file. A file written by a
// little-endian host starts with the bytes d4 c3 b2 a1, so it is viewed with
// struct_view_le.
using pcap_file_header =
  s2s::struct_field_list<
    s2s::magic_number<"magic", u32, 4_B, 0xa1b2c3d4>,
    s2s::basic_field<"version_major", u16, 2_B>,
    s2s::basic_field<"version_minor", u16, 2_B>,
    s2s::basic_field<"thiszone", u32, 4_B>,
    s2s::basic_field<"sigfigs", u32, 4_B>,
    s2s::basic_field<"snaplen", u32, 4_B>,
    s2s::basic_field<"network", u32, 4_B>
  >;

// The 16-byte header in front of every captured packet. incl_len says how many
// payload bytes follow, which is why the walk below cannot be a fixed stride.
using pcap_record_header =
  s2s::struct_field_list<
    s2s::basic_field<"ts_sec", u32, 4_B>,
    s2s::basic_field<"ts_usec", u32, 4_B>,
    s2s::basic_field<"incl_len", u32, 4_B>,
    s2s::basic_field<"orig_len", u32, 4_B>
  >;

constexpr unsigned char capture_bytes[] = {
  0xd4, 0xc3, 0xb2, 0xa1, 0x02, 0x00, 0x04, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  // first packet: 3 bytes captured of 60
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0xaa, 0xbb, 0xcc,
  // second packet: 5 bytes captured of 5
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x01, 0x02, 0x03, 0x04, 0x05
};

auto write_capture(const char* path) -> bool {
  std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char*>(capture_bytes), sizeof(capture_bytes));
  return static_cast<bool>(out);
}

// Adds up the captured bytes of every packet. Only incl_len is read from each
// record header, so the other three fields are never decoded; the payload is
// skipped by arithmetic, never looked at.
auto captured_bytes(std::span<const char> file) -> std::expected<u32, s2s::cast_error> {
  u32 total = 0;
  std::size_t offset = s2s::wire_width<pcap_file_header>;
  while(offset < file.size()) {
    const auto record = s2s::struct_view_le<pcap_record_header>(file.subspan(offset));
    if(!record)
      return std::unexpected(record.error());

    const u32 incl_len = (*record)["incl_len"_f];
    total += incl_len;
    offset += s2s::wire_width<pcap_record_header> + incl_len;
  }
  return total;
}

auto main() -> int {
  if(!write_capture("capture.pcap"))
    return 1;

  std::ifstream file("capture.pcap", std::ios::in | std::ios::binary);
  // The view borrows these bytes: data has to outlive every view made from it.
  const std::vector<char> data{std::istreambuf_iterator<char>(file),
                               std::istreambuf_iterator<char>()};

  // A dynamic extent: the length is checked once, here, and so is the magic.
  const auto header = s2s::struct_view_le<pcap_file_header>(std::span<const char>(data));
  if(!header || (*header)["network"_f] != 1 || (*header)["snaplen"_f] != 0xffff)
    return 1;

  const auto total = captured_bytes(data);
  if(!total || *total != 8)
    return 1;

  // A capture cut off inside a record header names the field that does not fit.
  const std::span<const char> cut_off(data.data(), s2s::wire_width<pcap_file_header> + 16 + 3 + 10);
  const auto failed = captured_bytes(cut_off);
  if(failed.has_value())
    return 1;

  return failed.error().failure_reason == s2s::error_reason::buffer_exhaustion
      && failed.error().failed_at == std::string_view{"incl_len"} ? 0 : 1;
}
// docs-end
