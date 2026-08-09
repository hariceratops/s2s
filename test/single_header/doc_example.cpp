// The worked example and the derived-field claims from UserGuide.md's
// "Writing to stream" section, kept compilable so the documentation cannot
// drift away from the library without something failing.
#include "s2s.hpp"

#include <sstream>
#include <vector>
#include <utility>
#include <cstdio>

using namespace s2s_literals;
using u16 = unsigned short;
using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"count">>>
  >;

auto main() -> int {
  our_struct obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344};
  // "count" is never assigned. It is derived from data.size().

  // The guide states reads give the stored slot, not the derived value.
  if(obj["count"_f] != 0) {
    std::printf("a derived slot is not populated before a write\n");
    return 1;
  }
  const auto through_const = std::as_const(obj)["count"_f];
  if(through_const != 0)
    return 1;

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  if(!s2s::stream_cast_le<our_struct>(le, obj))
    return 1;

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  if(!s2s::stream_cast_be<our_struct>(be, obj))
    return 1;

  auto back = s2s::struct_cast_be<our_struct>(be);
  if(!back || (*back)["count"_f] != 2)
    return 1;

  // The guide's replacement for the no-longer-compiling `["count"] = 5`.
  auto parsed = *back;
  parsed["data"_f] = std::vector<u16>(5);
  std::stringstream again(std::ios::in | std::ios::out | std::ios::binary);
  if(!s2s::stream_cast_be<our_struct>(again, parsed))
    return 1;
  auto reread = s2s::struct_cast_be<our_struct>(again);
  if(!reread || (*reread)["count"_f] != 5) {
    std::printf("assigning the container did not move the derived length\n");
    return 1;
  }

  std::printf("doc example ok\n");
  return 0;
}
