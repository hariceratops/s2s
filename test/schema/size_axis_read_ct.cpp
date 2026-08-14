// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// The size callables are namespace scope on purpose. They are NTTPs of the
// schema, not captures of the test body.

#include <array>
#include <cstddef>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

auto area_of = [](auto rows, auto cols) { return rows * cols; };

using computed =
  s2s::struct_field_list<
    s2s::basic_field<"rows", u32, 4_B>,
    s2s::basic_field<"cols", u32, 4_B>,
    s2s::vec_field<
      "cells",
      u16,
      s2s::len_from_fields<area_of, "rows", "cols">
    >
  >;

// One length field, two dependents.
using fanout =
  s2s::struct_field_list<
    s2s::basic_field<"len", u32, 4_B>,
    s2s::vec_field<"a", u16, s2s::len_from_field<"len">>,
    s2s::vec_field<"b", u32, s2s::len_from_field<"len">>
  >;

auto main() -> int {
  // The element count is nowhere on the wire — it exists only as rows * cols.
  "a length computed from two fields sizes the container"_test = [] constexpr {
    std::array<u8, 16> buffer{
      0x02, 0x00, 0x00, 0x00,
      0x02, 0x00, 0x00, 0x00,
      0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<computed>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["rows"_f], 2u));
    expect(eq((*res)["cols"_f], 2u));
    expect(eq((*res)["cells"_f].size(), std::size_t{4}));
    expect(eq((*res)["cells"_f][0], u16{0x1122}));
    expect(eq((*res)["cells"_f][3], u16{0x7788}));
  };

  // Changing one source has to change the count, or the callable is not being
  // applied and something else is supplying the length.
  "changing a source changes the computed length"_test = [] constexpr {
    std::array<u8, 14> buffer{
      0x03, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00,
      0x22, 0x11, 0x44, 0x33, 0x66, 0x55
    };
    memstream<14> stream(buffer);

    auto res = s2s::struct_cast_le<computed>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["cells"_f].size(), std::size_t{3}));
  };

  "a computed length longer than the buffer fails on the container"_test = [] constexpr {
    std::array<u8, 12> buffer{
      0x02, 0x00, 0x00, 0x00,
      0x02, 0x00, 0x00, 0x00,
      0x22, 0x11, 0x44, 0x33
    };
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<computed>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::buffer_exhaustion));
    expect(eq(res.error().failed_at, std::string_view{"cells"}));
  };

  // Both dependents take their count from the same field, and the elements
  // differ in width, so a shared length is a count and not a byte size.
  "one length field sizes several dependents"_test = [] constexpr {
    std::array<u8, 16> buffer{
      0x02, 0x00, 0x00, 0x00,
      0x22, 0x11, 0x44, 0x33,
      0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<16> stream(buffer);

    auto res = s2s::struct_cast_le<fanout>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f].size(), std::size_t{2}));
    expect(eq((*res)["b"_f].size(), std::size_t{2}));
    expect(eq((*res)["a"_f][0], u16{0x1122}));
    expect(eq((*res)["b"_f][0], 0xdeadbeefu));
  };

  // test/must_not_compile/hidden_length_target.cpp asserts that naming a
  // length target does not compile. This is the other half: the length is
  // still read off the wire and still sizes its container — it is only the
  // user's view of it that is gone.
  "a length target sizes its container without being nameable"_test = [] constexpr {
    using derived =
      s2s::struct_field_list<
        s2s::basic_field<"len", u32, 4_B>,
        s2s::str_field<"str", s2s::len_from_field<"len">>
      >;

    std::array<u8, 7> buffer{0x03, 0x00, 0x00, 0x00, 'a', 'b', 'c'};
    memstream<7> stream(buffer);

    auto res = s2s::struct_cast_le<derived>(stream);

    expect(eq(res.has_value(), true));
    const auto fields = *res;
    expect(eq(fields["str"_f].size(), std::size_t{3}));
    expect(eq(std::string_view{fields["str"_f]}, std::string_view{"abc"}));
  };

  // The container a length target sizes is ordinary assignable data — only
  // the target itself is hidden. Nothing here reads "len": there is no
  // subscript that reaches it, on a const object or otherwise.
  "the container behind a length target stays assignable"_test = [] constexpr {
    using derived_len =
      s2s::struct_field_list<
        s2s::basic_field<"len", u32, 4_B>,
        s2s::str_field<"str", s2s::len_from_field<"len">>
      >;

    derived_len obj{};
    obj["str"_f] = "hello";

    expect(eq(std::string_view{obj["str"_f]}, std::string_view{"hello"}));
    expect(eq(std::string_view{std::as_const(obj)["str"_f]}, std::string_view{"hello"}));
  };

  "a derived discriminant is readable while its union stays assignable"_test = [] constexpr {
    using alt_1 = s2s::struct_field_list<s2s::basic_field<"x", u32, 4_B>>;
    using alt_2 = s2s::struct_field_list<s2s::basic_field<"y", u32, 4_B>>;
    using tagged =
      s2s::struct_field_list<
        s2s::basic_field<"tag", u32, 4_B>,
        s2s::variance<"body", s2s::type<
          s2s::match_field<"tag">,
          s2s::type_switch<
            s2s::match_case<0xcafed00d, s2s::as_struct<alt_1>>,
            s2s::match_case<0xdeadbeef, s2s::as_struct<alt_2>>
          >
        >>
      >;

    tagged obj{};
    // "body" is not derived — the discriminant is derived *from* it.
    obj["body"_f] = alt_1{};

    expect(eq(std::as_const(obj)["tag"_f], 0u));
    expect(eq(obj["tag"_f], 0u));
    expect(eq(std::holds_alternative<alt_1>(obj["body"_f]), true));
  };

  // size_dont_care: the width comes from the nested schema rather than the
  // size axis, so the field behind it starts where that schema ended.
  "a size_dont_care field occupies exactly its nested schema"_test = [] constexpr {
    using point =
      s2s::struct_field_list<
        s2s::basic_field<"x", u16, 2_B>,
        s2s::basic_field<"y", u16, 2_B>
      >;
    using schema =
      s2s::struct_field_list<
        s2s::struct_field<"inner", point>,
        s2s::basic_field<"tail", u16, 2_B>
      >;

    std::array<u8, 6> buffer{0x22, 0x11, 0x44, 0x33, 0x66, 0x55};
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast_le<schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["inner"_f]["y"_f], u16{0x3344}));
    expect(eq((*res)["tail"_f], u16{0x5566}));
  };

  // The claim the terse surface rests on: an omitted size means sizeof(T) on
  // the wire.
  "a basic_field with no size occupies sizeof(T)"_test = [] constexpr {
    using defaulted =
      s2s::struct_field_list<
        s2s::basic_field<"a", u16>,
        s2s::basic_field<"b", u16>
      >;

    std::array<u8, 4> buffer{0x22, 0x11, 0x44, 0x33};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<defaulted>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], u16{0x1122}));
    expect(eq((*res)["b"_f], u16{0x3344}));
  };

  // Three is the width that pins the claim: not the type's size, not half of
  // it, and no power of two to fall back on. What it really guards is the
  // field after it — a width silently rounded up to sizeof(T) would still
  // decode "v" correctly and put "tail" one byte late.
  "an odd byte count is not rounded up to the type's width"_test = [] constexpr {
    using odd_width =
      s2s::struct_field_list<
        s2s::basic_field<"v", u32, 3_B>,
        s2s::basic_field<"tail", u8>
      >;

    std::array<u8, 4> buffer{0x33, 0x22, 0x11, 0x7f};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<odd_width>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["v"_f], 0x112233u));
    expect(eq((*res)["tail"_f], u8{0x7f}));
  };

  // A width narrower than the type is the case an omitted size cannot express,
  // and the one 2_B still exists for.
  "a byte-count literal narrower than the type is honoured"_test = [] constexpr {
    using narrowed =
      s2s::struct_field_list<
        s2s::basic_field<"a", u32, 2_B>,
        s2s::basic_field<"b", u16>
      >;

    std::array<u8, 4> buffer{0x22, 0x11, 0x44, 0x33};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<narrowed>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["a"_f], 0x1122u));
    expect(eq((*res)["b"_f], u16{0x3344}));
  };
}
