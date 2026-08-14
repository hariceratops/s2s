// Backs the reference stream implementation in docs/streams.md.
// The region between docs-begin and docs-end is compared against that fenced
// block by the doc_examples_match test, so edit them together or the check
// fails.
// docs-begin
#include "s2s.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <ios>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
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

// A heartbeat frame, small enough that the stream is the interesting part.
using heartbeat =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x48, 0x42}>,
    s2s::basic_field<"sequence", u32, 4_B>
  >;

static_assert(s2s::input_stream_like<byte_stream>);
static_assert(s2s::output_stream_like<byte_stream>);

auto main() -> int {
  heartbeat beat{};
  beat["marker"_f] = std::array<u8, 2>{0x48, 0x42};
  beat["sequence"_f] = 0xdeadbeefu;

  byte_stream stream;

  const auto round_tripped =
    s2s::stream_cast_be<heartbeat>(stream, beat)
      .and_then([&stream] { return s2s::struct_cast_be<heartbeat>(stream); })
      .transform([](const heartbeat& parsed) {
        return parsed["sequence"_f] == 0xdeadbeefu;
      });

  if(!round_tripped.value_or(false))
    return 1;

  // Reading past the end sets the bad flag, which surfaces as buffer_exhaustion.
  const auto overrun = s2s::struct_cast_be<heartbeat>(stream);
  if(overrun.has_value())
    return 1;

  return overrun.error().failure_reason == s2s::error_reason::buffer_exhaustion ? 0 : 1;
}
// docs-end
