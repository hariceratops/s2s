// Consumers get exactly one file. This target includes only that file — no
// -I include — so a header that silently depends on the source tree fails
// here rather than in someone else's build.
#include "s2s.hpp"

#include <sstream>
#include <cstdio>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using record =
  s2s::struct_field_list<
    s2s::basic_field<"x", u16, 2_B>
  >;

// One schema touching every dispatch path the write side has: magic, derived
// length, variable buffer, fixed aggregate, nested record and a union with a
// derived discriminant.
using schema =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"count", u32, 4_B>,
    s2s::vec_field<"data", u16, s2s::len_from_field<"count">>,
    s2s::fixed_array_field<"pair", u16, 2>,
    s2s::struct_field<"nested", record>,
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<record>>
        >
      >
    >
  >;

auto populated() -> schema {
  record inner{};
  inner["x"_f] = 0x5566;

  schema obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344};
  obj["pair"_f] = std::array<u16, 2>{0x7788, 0x99aa};
  obj["nested"_f] = inner;
  obj["body"_f] = inner;
  return obj;
}

template <bool big_endian>
auto roundtrips() -> bool {
  const auto original = populated();
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);

  const auto written = big_endian ? s2s::stream_cast_be<schema>(stream, original)
                                  : s2s::stream_cast_le<schema>(stream, original);
  if(!written) {
    std::printf("write failed at %s\n", written.error().failed_at.data());
    return false;
  }

  const auto back = big_endian ? s2s::struct_cast_be<schema>(stream)
                               : s2s::struct_cast_le<schema>(stream);
  if(!back) {
    std::printf("read failed at %s\n", back.error().failed_at.data());
    return false;
  }

  return (*back)["data"_f] == std::vector<u16>{0x1122, 0x3344}
      && (*back)["pair"_f] == std::array<u16, 2>{0x7788, 0x99aa}
      && (*back)["nested"_f]["x"_f] == 0x5566
      && (*back)["tag"_f] == 0xdeadbeef
      && std::get<record>((*back)["body"_f])["x"_f] == 0x5566;
}

auto main() -> int {
  if(!roundtrips<false>()) {
    std::printf("little-endian round-trip failed\n");
    return 1;
  }
  if(!roundtrips<true>()) {
    std::printf("big-endian round-trip failed\n");
    return 1;
  }
  std::printf("single-header round-trip ok\n");
  return 0;
}
