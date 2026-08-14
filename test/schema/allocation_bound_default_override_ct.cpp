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

auto main() -> int {
  // TODO(047): lowering the macro lowers every undeclared field's ceiling. A
  // length accepted under the 16 MiB default must be rejected here, which is
  // what proves the default is actually read from the macro rather than baked
  // in at some other layer.
  //
  // Placeholder bodies: assert nothing yet, since the macro does not exist.
  "the macro sets the ceiling for a field declaring no bound"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): the invariant that matters most in this file. A declared
  // max_bytes is the author's intent; the macro is the library's guess. No
  // build-time setting may discard the former. Declare max_bytes above the
  // lowered default and confirm it is honoured, not clamped down to it.
  "a declared bound is not clamped by the macro"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): and the converse — a declared bound *below* the macro is also
  // honoured, so the two are not being max'd or min'd together by accident.
  "a declared bound below the macro is honoured"_test = [] constexpr {
    expect(eq(true, true));
  };
}
