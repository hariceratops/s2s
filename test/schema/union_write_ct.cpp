// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <cstddef>
#include <variant>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using alt_1 =
  s2s::struct_field_list<
    s2s::basic_field<"x", u16, 2_B>
  >;
using alt_2 =
  s2s::struct_field_list<
    s2s::basic_field<"y", u16, 2_B>
  >;

using union_struct =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
          s2s::match_case<0xdeadbeef, s2s::as_struct<alt_2>>
        >
      >
    >
  >;

// The two record tags 048 makes nameable, on the write path. Their read-side
// twins are in union_read_ct.cpp; together they are the round-trip 048 owes for
// tags that were documented but had never been instantiable.
using with_record_arr =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0xcafed00d, s2s::as_arr_of_records<alt_1, 2>>
        >
      >
    >
  >;


constexpr auto alt_1_with(u16 x) -> alt_1 {
  alt_1 held{};
  held["x"_f] = x;
  return held;
}

constexpr auto held_alt_2() -> union_struct {
  alt_2 held{};
  held["y"_f] = 0x1122;
  union_struct obj{};
  obj["body"_f] = held;
  return obj;
}

auto main() -> int {
  "little endian unions round trip"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    expect(eq(s2s::stream_cast_le<union_struct>(stream, held_alt_2()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<union_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["tag"_f], 0xdeadbeefu));
    expect(eq(std::get<alt_2>((*res)["body"_f])["y"_f], u16{0x1122}));
  };

  "big endian unions round trip"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    expect(eq(s2s::stream_cast_be<union_struct>(stream, held_alt_2()).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_be<union_struct>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["tag"_f], 0xdeadbeefu));
    expect(eq(std::get<alt_2>((*res)["body"_f])["y"_f], u16{0x1122}));
  };

  // The discriminant is never stored — 0xdeadbeef comes from the match_case
  // belonging to the held alternative.
  "the discriminant is derived from the held alternative"_test = [] constexpr {
    std::array<u8, 6> buffer{};
    memstream<6> stream(buffer);

    expect(eq(s2s::stream_cast_be<union_struct>(stream, held_alt_2()).has_value(), true));
    constexpr std::array<u8, 6> expected{0xde, 0xad, 0xbe, 0xef, 0x11, 0x22};
    for(std::size_t idx = 0; idx < expected.size(); ++idx)
      expect(eq(buffer[idx], expected[idx]));
  };

  "an array-of-records alternative round trips"_test = [] constexpr {
    with_record_arr obj{};
    obj["body"_f] = std::array<alt_1, 2>{alt_1_with(0x1111), alt_1_with(0x2222)};

    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    expect(eq(s2s::stream_cast_le<with_record_arr>(stream, obj).has_value(), true));
    stream.rewind();
    auto res = s2s::struct_cast_le<with_record_arr>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<std::array<alt_1, 2>>((*res)["body"_f])[1]["x"_f], u16{0x2222}));
  };

  // No write case for as_vec_of_records: writing a length-prefixed container
  // alternative does not work today, and 048 found it rather than caused it.
  // The read side is covered in union_read_ct.cpp and passes.
  //
  // The obligation machinery has the agreement half but not the derivation
  // half. union_len_obligation::agrees (derived_value.hpp) compares the held
  // vector's size against the length field's value, but nothing derives that
  // value from a union-held container the way it is derived for a plain
  // vec_field. So "n" stays 0, agrees reports 2 != 0, and the write fails with
  // found_contradicting_length at "n" — and because "n" is a length target the
  // schema author cannot set it either, so there is no spelling that works.
  //
  // Unreachable before 048 because as_vec_of_records had no is_type_tag
  // specialization and could not be named. Needs its own issue; not widened
  // into this slice.
}
