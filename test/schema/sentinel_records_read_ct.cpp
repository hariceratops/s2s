// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// Every bound-related case declares a small max_bytes. The sentinel_not_found
// path at the *default* 16 MiB bound reads on the order of 10^5 nested records
// under constant evaluation, well past gcc's -fconstexpr-ops-limit; that path
// is exercised in sentinel_records_read.cpp (GoogleTest) only.

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

constexpr std::size_t bound_elements = 3;

using bounded_blocks =
  s2s::struct_field_list<
    s2s::vector_of_records<"blocks", sub_block,
      s2s::until_field_equals<"size", u8{0}>,
      s2s::max_bytes<bound_elements * sizeof(sub_block)>>
  >;

using signed_record =
  s2s::struct_field_list<
    s2s::basic_field<"v", signed char, 1_B>
  >;

using signed_run =
  s2s::struct_field_list<
    s2s::vector_of_records<"run", signed_record,
      s2s::until_field_equals<"v", 0xff>,
      s2s::max_bytes<8 * sizeof(signed_record)>>
  >;

auto main() -> int {
  "a GIF sub-block run drops the terminator and the next field reads on"_test = [] constexpr {
    std::array<u8, 11> buffer{0x08, 0x02, 'a', 'b', 0x01, 'c', 0x00, 0x22, 0x11, 0, 0};
    memstream<11> stream(buffer);

    auto res = s2s::struct_cast_le<image_data>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["lzw_min_code_size"_f], u8{8}));
    expect(eq((*res)["blocks"_f].size(), std::size_t{2}));
    expect(eq((*res)["blocks"_f][0]["data"_f].size(), std::size_t{2}));
    expect(eq((*res)["blocks"_f][1]["data"_f][0], u8{'c'}));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "a DNS label list reads labels up to the zero-length one"_test = [] constexpr {
    std::array<u8, 9> buffer{0x03, 'w', 'w', 'w', 0x01, 'a', 0x00, 0x22, 0x11};
    memstream<9> stream(buffer);

    auto res = s2s::struct_cast_le<dns_name>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["labels"_f].size(), std::size_t{2}));
    expect(eq(std::string_view{(*res)["labels"_f][0]["text"_f]}, std::string_view{"www"}));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "a terminator in first position yields an empty vector"_test = [] constexpr {
    std::array<u8, 5> buffer{0x08, 0x00, 0x22, 0x11, 0};
    memstream<5> stream(buffer);

    auto res = s2s::struct_cast_le<image_data>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["blocks"_f].empty(), true));
    expect(eq((*res)["tail"_f], u16{0x1122}));
  };

  "max_bytes accepts exactly the bound's worth of elements plus the terminator"_test = [] constexpr {
    std::array<u8, 7> buffer{0x01, 'a', 0x01, 'b', 0x01, 'c', 0x00};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_blocks>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["blocks"_f].size(), std::size_t{3}));
  };

  "max_bytes rejects one element more with sentinel_not_found"_test = [] constexpr {
    std::array<u8, 9> buffer{0x01, 'a', 0x01, 'b', 0x01, 'c', 0x01, 'd', 0x00};
    memstream<9> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_blocks>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::sentinel_not_found));
    expect(eq(res.error().failed_at, std::string_view{"blocks"}));
  };

  "a stream that runs dry between elements reports buffer_exhaustion"_test = [] constexpr {
    std::array<u8, 4> buffer{0x01, 'a', 0x01, 'b'};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_blocks>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
  };

  "a stream that runs dry mid-element reports buffer_exhaustion"_test = [] constexpr {
    std::array<u8, 3> buffer{0x01, 'a', 0x05};
    memstream<3> stream(buffer);

    auto res = s2s::struct_cast_le<bounded_blocks>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
  };

  "a signed field matches a sentinel of 0xff by casting the sentinel down"_test = [] constexpr {
    std::array<u8, 3> buffer{0x01, 0x02, 0xff};
    memstream<3> stream(buffer);

    auto res = s2s::struct_cast_le<signed_run>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["run"_f].size(), std::size_t{2}));
  };
}
