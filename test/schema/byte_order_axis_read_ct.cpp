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

// The same announcement written the other two ways. `type_deduction` supports
// three input forms and so does this axis; these two exist so a reader can see
// they resolve the same order over the same bytes.
constexpr auto first_marker_byte = [](std::array<u8, 2> marker) { return marker[0]; };

using tiff_header_computed =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<s2s::compute<first_marker_byte, u8, "marker">,
        s2s::order_switch<
          s2s::order_case<u8{'I'}, std::endian::little>,
          s2s::order_case<u8{'M'}, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;

constexpr auto marker_is_ii = [](std::array<u8, 2> m) { return m[0] == 'I' && m[1] == 'I'; };
constexpr auto marker_is_mm = [](std::array<u8, 2> m) { return m[0] == 'M' && m[1] == 'M'; };

using tiff_header_laddered =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<
        s2s::order_if_else<
          s2s::order_branch<s2s::predicate<marker_is_ii, "marker">, std::endian::little>,
          s2s::order_branch<s2s::predicate<marker_is_mm, "marker">, std::endian::big>>>>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;


// A UTF-16 byte order mark, read as its two bytes rather than as one u16 —
// which it has to be, since a u16 marker would itself be order-dependent. The
// order follows from both fields together, so no single match_field can express
// it: this is the case that justifies marking the record rather than a field.
using byte_order_mark =
  s2s::struct_field_list<
    s2s::basic_field<"first", u8, 1_B>,
    s2s::basic_field<"second", u8, 1_B>
  >;

constexpr auto bom_order = [](u8 first, u8 second) -> unsigned {
  if(first == 0xff && second == 0xfe) return 0u;
  if(first == 0xfe && second == 0xff) return 1u;
  return 2u;
};

using utf16_text =
  s2s::struct_field_list<
    s2s::announces_byte_order<"bom", byte_order_mark,
      s2s::order_from<s2s::compute<bom_order, unsigned, "first", "second">,
        s2s::order_switch<
          s2s::order_case<0u, std::endian::little>,
          s2s::order_case<1u, std::endian::big>>>>,
    s2s::fixed_array_field<"units", u16, 2>
  >;

constexpr auto bom_is_le = [](u8 first, u8 second) { return first == 0xff && second == 0xfe; };
constexpr auto bom_is_be = [](u8 first, u8 second) { return first == 0xfe && second == 0xff; };

using utf16_text_laddered =
  s2s::struct_field_list<
    s2s::announces_byte_order<"bom", byte_order_mark,
      s2s::order_from<
        s2s::order_if_else<
          s2s::order_branch<s2s::predicate<bom_is_le, "first", "second">, std::endian::little>,
          s2s::order_branch<s2s::predicate<bom_is_be, "first", "second">, std::endian::big>>>>,
    s2s::fixed_array_field<"units", u16, 2>
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

  "a ladder resolves the order from an announcing record"_test = [] constexpr {
    std::array<u8, 8> buffer{
      'M', 'M',
      0x00, 0x2a,
      0xca, 0xfe, 0xd0, 0x0d
    };
    memstream<8> stream(buffer);

    auto res = s2s::struct_cast<tiff_header_laddered>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic"_f], u16{42}));
    expect(eq((*res)["ifd_offset"_f], 0xcafed00du));
  };

  // Two fields, one order. 0xff 0xfe and 0xfe 0xff are the same two bytes in
  // the other order, so neither field decides anything on its own.
  "a callable resolves the order from two fields of the record"_test = [] constexpr {
    std::array<u8, 6> buffer{
      0xff, 0xfe,
      0x41, 0x00, 0x42, 0x00
    };
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast<utf16_text>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["units"_f][0], u16{0x41}));
    expect(eq((*res)["units"_f][1], u16{0x42}));
  };

  "the same two fields the other way round give the other order"_test = [] constexpr {
    std::array<u8, 6> buffer{
      0xfe, 0xff,
      0x00, 0x41, 0x00, 0x42
    };
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast<utf16_text>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["units"_f][0], u16{0x41}));
    expect(eq((*res)["units"_f][1], u16{0x42}));
  };

  "a ladder over two fields resolves what a single match cannot"_test = [] constexpr {
    std::array<u8, 6> buffer{
      0xfe, 0xff,
      0x00, 0x41, 0x00, 0x42
    };
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast<utf16_text_laddered>(stream);

    expect(eq(res.has_value(), true));
    expect(eq((*res)["units"_f][0], u16{0x41}));
    expect(eq((*res)["units"_f][1], u16{0x42}));
  };

  // The three forms are interchangeable where they express the same thing, and
  // this is the assertion that says so: one file, three schemas, identical
  // output. Both markers, because a form could agree on one and not the other.
  "all three forms produce identical reads"_test = [] constexpr {
    std::array<u8, 8> ii{'I', 'I', 0x2a, 0x00, 0x0d, 0xd0, 0xfe, 0xca};
    std::array<u8, 8> mm{'M', 'M', 0x00, 0x2a, 0xca, 0xfe, 0xd0, 0x0d};

    memstream<8> ii_matched(ii);
    memstream<8> ii_computed(ii);
    memstream<8> ii_laddered(ii);
    memstream<8> mm_matched(mm);
    memstream<8> mm_computed(mm);
    memstream<8> mm_laddered(mm);

    auto by_match_ii = s2s::struct_cast<tiff_header>(ii_matched);
    auto by_compute_ii = s2s::struct_cast<tiff_header_computed>(ii_computed);
    auto by_ladder_ii = s2s::struct_cast<tiff_header_laddered>(ii_laddered);
    auto by_match_mm = s2s::struct_cast<tiff_header>(mm_matched);
    auto by_compute_mm = s2s::struct_cast<tiff_header_computed>(mm_computed);
    auto by_ladder_mm = s2s::struct_cast<tiff_header_laddered>(mm_laddered);

    expect(eq(by_match_ii.has_value(), true));
    expect(eq((*by_compute_ii)["magic"_f], (*by_match_ii)["magic"_f]));
    expect(eq((*by_ladder_ii)["magic"_f], (*by_match_ii)["magic"_f]));
    expect(eq((*by_compute_ii)["ifd_offset"_f], (*by_match_ii)["ifd_offset"_f]));
    expect(eq((*by_ladder_ii)["ifd_offset"_f], (*by_match_ii)["ifd_offset"_f]));

    expect(eq(by_match_mm.has_value(), true));
    expect(eq((*by_compute_mm)["magic"_f], (*by_match_mm)["magic"_f]));
    expect(eq((*by_ladder_mm)["magic"_f], (*by_match_mm)["magic"_f]));
    expect(eq((*by_compute_mm)["ifd_offset"_f], (*by_match_mm)["ifd_offset"_f]));
    expect(eq((*by_ladder_mm)["ifd_offset"_f], (*by_match_mm)["ifd_offset"_f]));
  };

  // A BOM matching neither orientation fails the same way a marker matching no
  // case does, whichever form declared it — the ladder falling off its end and
  // the switch matching nothing are one outcome.
  "a ladder that matches no branch fails as a validation failure"_test = [] constexpr {
    std::array<u8, 6> buffer{
      0xff, 0xff,
      0x41, 0x00, 0x42, 0x00
    };
    memstream<6> stream(buffer);

    auto res = s2s::struct_cast<utf16_text_laddered>(stream);

    expect(eq(res.has_value(), false));
    expect(eq(res.error().failure_reason, s2s::error_reason::validation_failure));
    expect(eq(res.error().failed_at, std::string_view{"bom"}));
  };

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
