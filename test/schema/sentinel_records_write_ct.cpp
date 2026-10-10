// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;

using sub_block =
  s2s::struct_field_list<
    s2s::basic_field<"size", u8, 1_B>,
    s2s::vec_field<"data", u8, s2s::len_from_field<"size">>
  >;

using image_data =
  s2s::struct_field_list<
    s2s::basic_field<"lzw_min_code_size", u8, 1_B>,
    s2s::vector_of_records<"blocks", sub_block, s2s::until_field_equals<"size", u8{0}>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

using label =
  s2s::struct_field_list<
    s2s::basic_field<"len", u8, 1_B>,
    s2s::str_field<"text", s2s::len_from_field<"len">>
  >;

using dns_name =
  s2s::struct_field_list<
    s2s::vector_of_records<"labels", label, s2s::until_field_equals<"len", u8{0}>>,
    s2s::basic_field<"tail", u16, 2_B>
  >;

auto main() -> int {
  "a GIF sub-block run is followed by one synthesised terminator byte"_test = [] constexpr {
    std::array<u8, 9> buffer{};
    memstream<9> stream(buffer);
    image_data obj{};
    obj["lzw_min_code_size"_f] = u8{8};
    obj["blocks"_f].resize(2);
    obj["blocks"_f][0]["data"_f] = {'a', 'b'};
    obj["blocks"_f][1]["data"_f] = {'c'};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<image_data>(stream, obj);

    expect(eq(written.has_value(), true));
    constexpr std::array<u8, 9> expected{0x08, 0x02, 'a', 'b', 0x01, 'c', 0x00, 0x22, 0x11};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };

  "an empty vector emits only the terminator and reads back empty"_test = [] constexpr {
    std::array<u8, 4> buffer{};
    memstream<4> stream(buffer);
    image_data obj{};
    obj["lzw_min_code_size"_f] = u8{8};
    obj["tail"_f] = u16{0x1122};

    auto written = s2s::stream_cast_le<image_data>(stream, obj);
    expect(eq(written.has_value(), true));
    expect(eq(buffer[1], u8{0}));

    stream.rewind();
    auto res = s2s::struct_cast_le<image_data>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["blocks"_f].empty(), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "a GIF run round trips"_test = [] constexpr {
    std::array<u8, 9> buffer{0x08, 0x02, 'a', 'b', 0x01, 'c', 0x00, 0x22, 0x11};
    memstream<9> in(buffer);
    auto read = s2s::struct_cast_le<image_data>(in);
    expect(eq(read.has_value(), true));

    std::array<u8, 9> out{};
    memstream<9> stream(out);
    auto written = s2s::stream_cast_le<image_data>(stream, *read);

    expect(eq(written.has_value(), true));
    for(std::size_t idx = 0; idx < buffer.size(); ++idx)
      expect(eq(out[idx], buffer[idx]));
  };

  "a DNS label list round trips"_test = [] constexpr {
    std::array<u8, 9> buffer{0x03, 'w', 'w', 'w', 0x01, 'a', 0x00, 0x22, 0x11};
    memstream<9> in(buffer);
    auto read = s2s::struct_cast_le<dns_name>(in);
    expect(eq(read.has_value(), true));

    std::array<u8, 9> out{};
    memstream<9> stream(out);
    auto written = s2s::stream_cast_le<dns_name>(stream, *read);

    expect(eq(written.has_value(), true));
    for(std::size_t idx = 0; idx < buffer.size(); ++idx)
      expect(eq(out[idx], buffer[idx]));
  };
}
