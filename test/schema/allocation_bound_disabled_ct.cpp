// The off switch: S2S_DEFAULT_MAX_BYTES set to SIZE_MAX.
//
// This is the escape hatch chosen instead of building a separate on/off profile
// — if it does not work, the argument for not needing a second mechanism is
// wrong. It is a whole translation unit because a macro is per-TU; the paired
// file allocation_bound_default_override_ct.cpp sets a *low* ceiling, and the
// two together show the same schema and the same bytes producing different
// outcomes purely from the macro.
//
// The discriminating case is deliberately small in absolute terms: with the
// defaults off, the read proceeds and then fails at the stream, so the length
// must be one that is safe to actually allocate. Nothing here should allocate
// more than a few hundred bytes.
#define S2S_DEFAULT_MAX_BYTES (std::numeric_limits<std::size_t>::max())

#include <array>
#include <cstddef>
#include <limits>
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
  // TODO(047): the same schema and the same short stream that
  // allocation_bound_default_override_ct.cpp rejects with excessive_length must
  // here get past the bound and fail at the stream instead — buffer_exhaustion,
  // not excessive_length. The changed *reason* is the observable; a test that
  // only checked "still fails" would pass with the macro ignored entirely.
  //
  // Placeholder body: asserts nothing yet, since the macro does not exist.
  "with defaults off a length is not rejected before the read"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): a declared max_bytes still bites with the defaults off. This is
  // the same invariant as its twin in the lowered TU, from the other side: no
  // build-time setting discards a limit the author wrote.
  "a declared bound still applies with the defaults off"_test = [] constexpr {
    expect(eq(true, true));
  };
}
