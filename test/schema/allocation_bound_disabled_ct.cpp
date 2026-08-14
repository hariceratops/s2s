// The off switch: S2S_DEFAULT_MAX_BYTES set to the maximum.
//
// This is the escape hatch chosen instead of building a separate on/off
// profile — if it does not work, the argument for not needing a second
// mechanism is wrong. It is a whole translation unit because a macro is
// per-TU; allocation_bound_default_override_ct.cpp sets a *low* ceiling, and
// the two together show the same schema and the same bytes producing different
// outcomes purely from the macro.
//
// Spelled with ~size_t{0} rather than std::numeric_limits, so the macro does
// not depend on <limits> having been included before the library header.
#define S2S_DEFAULT_MAX_BYTES (~std::size_t{0})

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

using declared =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"v", u8, s2s::len_from_field<"n">, s2s::max_bytes<16>>
  >;

auto main() -> int {
  // The same schema and the same short stream that the lowered TU rejects with
  // excessive_length gets past the bound here and fails at the stream instead.
  // The changed *reason* is the observable — a test asserting only "still
  // fails" would pass with the macro ignored entirely. The length is small
  // enough to be safe to actually allocate, since nothing rejects it now.
  "with defaults off a length is not rejected before the read"_test = [] constexpr {
    std::array<u8, 4> buffer{0x64, 0x00, 0x00, 0x00};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<undeclared>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::buffer_exhaustion));
  };

  // A declared bound still bites with the defaults off — the same invariant as
  // its twin in the lowered TU, from the other side.
  "a declared bound still applies with the defaults off"_test = [] constexpr {
    std::array<u8, 4> buffer{0x64, 0x00, 0x00, 0x00};
    memstream<4> stream(buffer);

    auto res = s2s::struct_cast_le<declared>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::excessive_length));
  };
}
