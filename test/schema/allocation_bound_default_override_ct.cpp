// The global knob: S2S_DEFAULT_MAX_BYTES.
//
// The macro is guarded by #ifndef and s2s is header-only, so a translation unit
// can choose its own ceiling simply by defining it before the include. That is
// why this needs no CMake variant and no target_compile_definitions — the
// define below is the entire mechanism under test.
//
// 64 bytes is deliberately absurd: it makes an over-bound case expressible in a
// handful of wire bytes, so the tests stay readable and stay constexpr.
#define S2S_DEFAULT_MAX_BYTES 64u

#include <array>
#include <cstddef>
#include <string_view>
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

using undeclared =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u8, s2s::len_from_field<"n">>
  >;

// Declared above the lowered default, which the macro must not clamp.
using declared_higher =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u8, s2s::len_from_field<"n">, s2s::max_bytes<128>>
  >;

// Declared below it, which the macro must not raise either.
using declared_lower =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u8, s2s::len_from_field<"n">, s2s::max_bytes<16>>
  >;

auto main() -> int {
  // 100 bytes is under the 16 MiB shipped default and over this file's 64, so
  // a rejection here can only come from the macro being read.
  "the macro sets the ceiling for a field declaring no bound"_test = [] constexpr {
    expect(eq(s2s::default_max_bytes, std::size_t{64}));

    std::array<u8, 4> buffer{0x64, 0x00, 0x00, 0x00};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<undeclared>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };

  // The invariant that matters most here. A declared max_bytes is the author's
  // intent; the macro is the library's guess. No build-time setting discards
  // the former — the same 100 bytes that the default rejects are read.
  "a declared bound is not clamped by the macro"_test = [] constexpr {
    std::array<u8, 104> buffer{0x64, 0x00, 0x00, 0x00};
    memstream<104> stream(buffer);

    auto res = s2s::struct_cast_le<declared_higher>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["v"_f].size(), std::size_t{100}));
  };

  // And the converse, so the two are not being max'd together by accident: a
  // declared bound below the macro still bites at its own number, not at 64.
  "a declared bound below the macro is honoured"_test = [] constexpr {
    std::array<u8, 24> buffer{0x14, 0x00, 0x00, 0x00};
    memstream<24> stream(buffer);

    auto res = s2s::struct_cast_le<declared_lower>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };
}
