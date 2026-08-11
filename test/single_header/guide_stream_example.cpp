// Backs the reference stream implementation in UserGuide.md's Streams section.
// The region between docs-begin and docs-end is compared against that fenced
// block by the doc_examples_match test, so edit them together or the check
// fails.
// docs-begin
#include "s2s.hpp"

#include <algorithm>
#include <cstddef>
#include <ios>
#include <vector>

using namespace s2s_literals;

using u32 = unsigned int;

// A complete stream, satisfying read_trait, write_trait and convertible_to_bool.
// Nothing else is required: no seeking, no exceptions, no std::ios base.
class byte_stream {
  std::vector<char> buffer;
  std::size_t read_pos{0};
  bool ok{true};

public:
  // read_trait: takes a raw destination and a count, returns *this by reference.
  auto read(char* dest, std::streamsize size_to_read) -> byte_stream& {
    const auto n = static_cast<std::size_t>(size_to_read);
    if(read_pos + n > buffer.size()) {
      ok = false;               // report exhaustion by going bad, not by throwing
      return *this;
    }
    std::copy_n(buffer.begin() + static_cast<std::ptrdiff_t>(read_pos), n, dest);
    read_pos += n;
    return *this;
  }

  // write_trait: takes a raw source and a count, returns *this by reference.
  auto write(const char* src, std::size_t size_to_write) -> byte_stream& {
    buffer.insert(buffer.end(), src, src + size_to_write);
    return *this;
  }

  // convertible_to_bool: the library tests the stream after every access.
  explicit operator bool() const { return ok; }
};

using our_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>
  >;

static_assert(s2s::input_stream_like<byte_stream>);
static_assert(s2s::output_stream_like<byte_stream>);

auto main() -> int {
  our_struct obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["a"_f] = 0xdeadbeefu;

  byte_stream stream;
  if(const auto written = s2s::struct_write_be<our_struct>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<our_struct>(stream);
  if(!back || (*back)["a"_f] != 0xdeadbeefu)
    return 1;

  // Reading past the end sets the bad flag, which surfaces as buffer_exhaustion.
  const auto overrun = s2s::struct_cast_be<our_struct>(stream);
  return !overrun
      && overrun.error().failure_reason == s2s::error_reason::buffer_exhaustion
        ? 0 : 1;
}
// docs-end
