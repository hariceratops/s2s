// S2S_DEFAULT_MAX_BYTES against a bound declared on a union alternative.
//
// Its own translation unit for the reason allocation_bound_default_override_ct
// is one: the macro is #ifndef-guarded and s2s is header-only, so a file
// chooses its own ceiling by defining it before the include, and no two
// ceilings can coexist in one TU. That file makes the claim for descriptors;
// this one makes it for tags, because the two reach `field_bound` by different
// routes — a descriptor through bound_of_pack directly, an alternative through
// the tag and to_field_choices — and only one of those was ever tested.
//
// 64 bytes is deliberately absurd: it makes an over-bound case expressible in a
// handful of wire bytes, so the tests stay readable and stay constexpr.
#define S2S_DEFAULT_MAX_BYTES 64u

#include <array>
#include <cstddef>
#include <string_view>
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
using u32 = unsigned int;

// 0x01 declares nothing and takes the lowered default; 0x02 declares above it
// and 0x03 below it, so the macro is tested in both directions against a
// declaration.
using alternatives =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_vec<u8, s2s::len_from_field<"n">>>,
          s2s::match_case<0x02, s2s::as_string<s2s::len_from_field<"n">,
                                               s2s::max_bytes<128>>>,
          s2s::match_case<0x03, s2s::as_vec<u32, s2s::len_from_field<"n">,
                                            s2s::max_bytes<16>>>
        >
      >
    >
  >;

auto main() -> int {
  // 100 bytes is under the 16 MiB shipped default and over this file's 64, so a
  // rejection here can only come from the macro being read.
  "the macro sets the ceiling for an alternative declaring no bound"_test = [] constexpr {
    expect(eq(s2s::default_max_bytes, std::size_t{64}));

    std::array<u8, 8> buffer{0x01, 0x00, 0x00, 0x00,
                             0x64, 0x00, 0x00, 0x00};
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast_le<alternatives>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // The invariant that matters most. A declared max_bytes is the author's
  // intent and the macro is the library's guess, so the same 100 bytes the
  // default rejects are read when the alternative declares room for them.
  "a bound declared on an alternative is not clamped by the macro"_test = [] constexpr {
    std::array<u8, 108> buffer{0x02, 0x00, 0x00, 0x00,
                               0x64, 0x00, 0x00, 0x00};
    memstream<108> stream(buffer);

    auto res = s2s::struct_cast_le<alternatives>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<std::string>((*res)["body"_f]).size(), std::size_t{100}));
  };

  // And the converse, so the two are not being max'd together by accident: a
  // declared bound below the macro still bites at its own number, not at 64.
  // 16 bytes admits four u32s, so five is over.
  "a declared bound below the macro is honoured"_test = [] constexpr {
    std::array<u8, 28> buffer{0x03, 0x00, 0x00, 0x00,
                              0x05, 0x00, 0x00, 0x00};
    memstream<28> stream(buffer);

    auto res = s2s::struct_cast_le<alternatives>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };
}
