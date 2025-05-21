#include <ios>
#include <iostream>
#include <expected>
#include <array>
#include "../include/s2s.hpp"
#include "constexpr_memstream.hpp"


constexpr auto comptime_memcpy(char* dest, char* src, std::size_t size_to_copy) {
  std::size_t idx{0};
  while(idx < size_to_copy) {
    dest[idx] = src[idx];
    idx++;
  }
}

using u32 = unsigned int;
using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
  >;


constexpr auto parse_our_struct() -> std::expected<our_struct, s2s::cast_error>
{
  std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
  memstream<8> stream(buffer);
  return s2s::struct_cast_le<our_struct>(stream);
}

constexpr auto failing_parse_our_struct() -> std::expected<our_struct, s2s::cast_error> 
{
  std::array<u8, 7> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe};
  memstream<7> stream(buffer);
  return s2s::struct_cast_le<our_struct>(stream);
}

constexpr auto res = parse_our_struct();
static_assert(res);
constexpr auto fields = *res;
static_assert(fields["a"_f] == 0xdeadbeef);
static_assert(fields["b"_f] == 0xcafed00d);

constexpr auto fail_res = failing_parse_our_struct();
static_assert(not fail_res);
constexpr auto err = fail_res.error();
static_assert(err.failed_at == "b");
static_assert(err.failure_reason == s2s::error_reason::buffer_exhaustion);


void test_001() {
  auto res = failing_parse_our_struct();
  if(res) {
    auto fields = *res;
    std::cout << std::hex << fields["a"_f] << " " << fields["b"_f] << '\n';
  } else {
    std::cout << static_cast<int>(res.error().failure_reason) << " " << res.error().failed_at << '\n';
  }
}

void test_002() {
  auto res = parse_our_struct();
  if(res) {
    auto fields = *res;
    std::cout << std::hex << fields["a"_f] << " " << fields["b"_f] << '\n';
  } else {
    std::cout << static_cast<int>(res.error().failure_reason) << " " << res.error().failed_at << '\n';
  }
}

using test_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::struct_field<
      "c", 
      s2s::struct_field_list<
        s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
        s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
      >
    >
  >;

constexpr auto parse_our_recs_struct() -> std::expected<test_struct, s2s::cast_error>
{
  std::array<u8, 16> buffer{
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
  };
  memstream<16> stream(buffer);
  return s2s::struct_cast_le<test_struct>(stream);
}

using records =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::basic_field<"y", u32, s2s::field_size<s2s::fixed<4>>>
  >;
using arr_recs_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::array_of_records<"records", records, 3>
  >;

constexpr auto parse_our_arr_recs_struct() -> std::expected<arr_recs_struct, s2s::cast_error>
{
  std::array<u8, 32> buffer{
    0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca
  };
  memstream<32> stream(buffer);
  return s2s::struct_cast_le<arr_recs_struct>(stream);
}

constexpr auto arr_recs_res = parse_our_arr_recs_struct();
static_assert(arr_recs_res);
constexpr auto arr_recs_fields = *arr_recs_res;
static_assert(arr_recs_fields["len"_f] == 3);
static_assert(arr_recs_fields["records"_f][0]["x"_f] == 0xdeadbeef);
static_assert(arr_recs_fields["records"_f][0]["y"_f] == 0xcafed00d);

using vec_recs_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::vector_of_records<"records", records, s2s::field_size<s2s::len_from_field<"len">>>
  >;

constexpr auto parse_our_vec_recs_struct() -> std::expected<vec_recs_struct, s2s::cast_error>
{
  std::array<u8, 32> buffer{
    0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca,
    0xef, 0xbe, 0xad, 0xde, 
    0x0d, 0xd0, 0xfe, 0xca
  };
  memstream<32> stream(buffer);
  return s2s::struct_cast_le<vec_recs_struct>(stream);
}

// will not work since vector "leaks" into runtime
// constexpr auto vec_recs_res = parse_our_vec_recs_struct();
// static_assert(vec_recs_res);
// constexpr auto vec_recs_fields = *vec_recs_res;
// static_assert(vec_recs_fields["len"_f] == 3);
// static_assert(vec_recs_fields["records"_f][0]["x"_f] == 0xdeadbeef);
// static_assert(vec_recs_fields["records"_f][0]["y"_f] == 0xcafed00d);

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
          s2s::match_case<0xcafed00d, s2s::struct_tag<inner_1>>,
          s2s::match_case<0xdeadbeef, s2s::struct_tag<inner_2>>
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


auto main(void) -> int {
  test_001();
  test_002();
  std::cout << sizeof(our_struct) << " " << sizeof(unionish) << " " << sizeof(opt_field_struct) << " " << sizeof(arr_recs_struct);
  return 0;
}

