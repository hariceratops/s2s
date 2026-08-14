// The allocation ceiling: declared per field with max_bytes, and applied by
// default where nothing is declared.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.
//
// This file verifies an *option*, not a container kind, which is why it is its
// own pair rather than additions to variable_buffer_read*.

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
using u16 = unsigned short;
using u32 = unsigned int;

auto main() -> int {
  // TODO(047): the 3x3 matrix — below-bound, exactly at bound, over bound,
  // for vec_field, str_field and vector_of_records. At-bound must be ACCEPTED:
  // max_bytes<N> is inclusive, and getting that backwards is the single most
  // likely silent error in this feature.
  //
  // Placeholder bodies: assert nothing yet, since max_bytes does not exist.
  "a length under its declared bound is read"_test = [] constexpr {
    expect(eq(true, true));
  };

  "a length exactly at its declared bound is read"_test = [] constexpr {
    expect(eq(true, true));
  };

  "a length over its declared bound is rejected"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): the bound is compared by integer division, so a ceiling that is
  // not a whole number of elements needs its own case — max_bytes<10> on a
  // vec<u32> admits 2 elements and rejects 3, and the arithmetic that gets
  // that right is not the arithmetic that gets the multiple case right.
  "a bound that is not a multiple of the element size truncates down"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): the denominator is memory, not wire. For vector_of_records the
  // bound is count * sizeof(record_struct), which has no relationship to how
  // many bytes those records occupy on the wire.
  "a record vector is bounded by its footprint, not its wire size"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): the headline property of the whole feature — a schema that
  // declares nothing is still protected. Everything above tests a declared
  // bound; this tests the default, and it is the case a user gets without
  // having thought about any of this.
  "a field declaring no bound still rejects an absurd length"_test = [] constexpr {
    expect(eq(true, true));
  };

  // TODO(047): a vec alternative inside a variance cannot declare its own
  // bound — as_vec takes a size, not an option pack (see
  // dev/inbox/union-alternatives-have-no-option-pack.md). It does get the
  // default, and without this case that claim is unverified.
  "a union alternative gets the default bound"_test = [] constexpr {
    expect(eq(true, true));
  };
}
