// The byte-order axis, read side — constant-evaluable tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Includes include/s2s.hpp rather than the amalgam, as every _ct file does.
// That is not incidental here: it is what discharges the design's finding 10 —
// the new public headers must be added to include/s2s.hpp's hand-maintained
// roster, and this file is the only thing in the suite that would notice if
// they were not. The amalgam picks them up automatically and so proves nothing.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

#include <array>
#include <bit>
#include <string_view>
#include <ut>

#include "../../include/s2s.hpp"
#include "../utils/constexpr_memstream.hpp"

using ut::expect;
using ut::eq;
using ut::operator""_test;
using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

using order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

// The shape acceptance is judged against: TIFF's II/MM header, followed by
// fields whose bytes only decode correctly under the order the header names.
using tiff_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;

auto main() -> int {
  // The whole claim of the feature: two different byte sequences, one schema,
  // one entry point taking no order, and the same values out of both.
  "an II header makes the rest of the file little-endian"_test = [] constexpr {
    std::array<u8, 8> buffer{
      'I', 'I',
      0x2a, 0x00,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast<tiff_header>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic"_f], u16{42}));
    expect(eq((*res)["ifd_offset"_f], 0xcafed00du));
  };

  "an MM header makes the rest of the file big-endian"_test = [] constexpr {
    std::array<u8, 8> buffer{
      'M', 'M',
      0x00, 0x2a,
      0xca, 0xfe, 0xd0, 0x0d
    };
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast<tiff_header>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic"_f], u16{42}));
    expect(eq((*res)["ifd_offset"_f], 0xcafed00du));
  };

  // A marker matching no case is a file that is not this format — a magic
  // mismatch, reported as one. Nothing is added for this case: the reader hands
  // back an ordinary rw_result and the fold attaches the announcing field's id.
  "a marker matching no case fails as a validation failure"_test = [] constexpr {
    std::array<u8, 8> buffer{
      'X', 'X',
      0x2a, 0x00,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast<tiff_header>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"byte_order"}));
  };

  // The announcing record's own fields are read before the order is known, so
  // they come off the stream at the seed the entry point supplies. A marker of
  // single bytes has no byte order to get wrong, which is what the compile-time
  // restriction on those fields (056) will make mandatory.
  "the announcing record's own fields are readable"_test = [] constexpr {
    std::array<u8, 8> buffer{
      'M', 'M',
      0x00, 0x2a,
      0xca, 0xfe, 0xd0, 0x0d
    };
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast<tiff_header>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["byte_order"_f]["marker"_f][0], u8{'M'}));
    expect(eq((*res)["byte_order"_f]["marker"_f][1], u8{'M'}));
  };

  // The control: a schema declaring no announcement is untouched by any of
  // this, and still reads through the fixed entry points.
  "a schema with no announcement still reads under struct_cast_le"_test = [] constexpr {
    using fixed_order_schema =
      s2s::struct_field_list<
        s2s::magic_number<"magic", u16, 2_B, 42>,
        s2s::basic_field<"ifd_offset", u32>
      >;

    std::array<u8, 6> buffer{
      0x2a, 0x00,
      0x0d, 0xd0, 0xfe, 0xca
    };
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast_le<fixed_order_schema>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic"_f], u16{42}));
    expect(eq((*res)["ifd_offset"_f], 0xcafed00du));
  };

  // TODO(055): the ladder form; a compute_t form over TWO fields of the
  // announcing record (the case a single match_field cannot express, and
  // therefore the one that justifies marking the record rather than the
  // field); and a three-way equivalence over the same bytes.
  // TODO(056): the over-reach guard — an order-dependent LATER sibling of the
  // containing record, asserted to read correctly. This matters more than the
  // negative cases: it proves the check does not extend past its boundary.
  // TODO(056): a magic_byte_array marker and a fixed_string marker are both
  // accepted.
  // TODO(057): announcing record one level deep, then two. Assert on a later
  // sibling OF THE CONTAINER and on a record after the container closes as
  // SEPARATE cases — one test covering both passes with the reference
  // threading half-broken.
}
