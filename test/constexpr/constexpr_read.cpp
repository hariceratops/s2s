#include <ios>
#include <iostream>
#include <expected>
#include <array>
#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"


constexpr auto comptime_memcpy(char* dest, char* src, std::size_t size_to_copy) {
  std::size_t idx{0};
  while(idx < size_to_copy) {
    dest[idx] = src[idx];
    idx++;
  }
}

using u32 = unsigned int;

auto is_a_eq_deadbeef = [](auto a){ return a == 0xdeadbeef; };
using opt_field_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::maybe<
      s2s::fixed_array_field<"c", u32, 3>, 
      s2s::parse_if<is_a_eq_deadbeef, s2s::with_fields<"a">>
    >
  >;

constexpr auto parse_opt_field_struct() -> std::expected<opt_field_struct, s2s::cast_error>
{
  std::array<u8, 20> buffer{
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xef, 0xbe
  };
  memstream<20> stream(buffer);
  return s2s::struct_cast_le<opt_field_struct>(stream);
}

constexpr auto opt_res = parse_opt_field_struct();
static_assert(opt_res);
constexpr auto opt_fields = *opt_res;
static_assert(opt_fields["a"_f] == 0xdeadbeef);
static_assert(opt_fields["b"_f] == 0xcafed00d);
static_assert(opt_fields["c"_f]);
constexpr auto arr = *opt_fields["c"_f];
static_assert(arr[0] == 0xdeadbeef);
static_assert(arr[1] == 0xcafed00d);
static_assert(arr[2] == 0xbeefbeef);


using inner_1 = 
 s2s::struct_field_list<
   s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>, 
   s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
>;
using inner_2 = 
 s2s::struct_field_list<
   s2s::basic_field<"p", u32, s2s::field_size<s2s::fixed<4>>>, 
   s2s::basic_field<"q", u32, s2s::field_size<s2s::fixed<4>>>
>;
using unionish = 
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>, 
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "c", 
      s2s::type<
        s2s::match_field<"a">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<inner_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<inner_2>>
        >
      >
    >
  >;


constexpr auto parse_union_field_struct() -> std::expected<unionish, s2s::cast_error>
{
  std::array<u8, 16> buffer{
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
  };
  memstream<16> stream(buffer);
  return s2s::struct_cast_le<unionish>(stream);
}

constexpr auto union_res = parse_union_field_struct();
static_assert(union_res);
constexpr auto union_fields = *union_res;
static_assert(union_fields["a"_f] == 0xdeadbeef);
static_assert(union_fields["b"_f] == 0xcafed00d);
constexpr auto inner_2_obj = std::get<inner_2>(union_fields["c"_f]);
static_assert(inner_2_obj["p"_f] == 0xdeadbeef);
static_assert(inner_2_obj["q"_f] == 0xcafed00d);


// The assertions above are all static_asserts; nothing is left to run. The
// trivial-field section moved to test/fields/trivial_read_ct.cpp, taking the
// test_001/test_002 debug printers with it — they asserted nothing.
auto main(void) -> int {
  return 0;
}

