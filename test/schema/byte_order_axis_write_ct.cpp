// The byte-order axis, write side — constant-evaluable tier.
// Spec:   dev/specs/byte-order-is-data-not-a-template-parameter.md
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Kept apart from byte_order_axis_read_ct rather than merged into one
// round-trip, per the tree's convention and because a round-trip stays green
// if both directions are wrong the same way — which, for a byte-order feature,
// is the likely failure. Every case here asserts the bytes, not just the
// values that come back.
//
// No `using namespace ut;` — see the read-side file.

#include <array>
#include <bit>
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

using order_guide =
  s2s::order_from<s2s::match_field<"marker">,
    s2s::order_switch<
      s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
      s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>;

using tiff_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker, order_guide>,
    s2s::magic_number<"magic", u16, 2_B, 42>,
    s2s::basic_field<"ifd_offset", u32>
  >;

// The marker is ordinary struct data and keeps its setter: assigning it is how
// a caller chooses the output order, which is why it is not a frozen field.
constexpr auto make_header(std::array<u8, 2> marker, u32 offset) -> tiff_header {
  tiff_header obj{};
  obj["byte_order"_f]["marker"_f] = marker;
  obj["ifd_offset"_f] = offset;
  return obj;
}

using tiff_ifd =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker, order_guide>,
    s2s::basic_field<"entry_count", u16, 2_B>
  >;

using nested_once =
  s2s::struct_field_list<
    s2s::struct_field<"ifd", tiff_ifd>,
    s2s::basic_field<"next_ifd", u32>
  >;

constexpr auto make_nested(std::array<u8, 2> marker) -> nested_once {
  nested_once obj{};
  obj["ifd"_f]["byte_order"_f]["marker"_f] = marker;
  obj["ifd"_f]["entry_count"_f] = 300;
  obj["next_ifd"_f] = 0xcafed00d;
  return obj;
}

// The composition with frozen-fields-write-their-own-value: an eq-constrained
// marker loses its setter, and what is left is a schema that can only ever
// write the one order 'M','M' decodes to. Nothing special-cases the other
// feature — the marker is read like any other field and happens to hold the
// value its constraint pins it to.
using frozen_marker =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{'M', 'M'}>
  >;

using always_big_endian =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", frozen_marker, order_guide>,
    s2s::basic_field<"value", u32>
  >;

auto main() -> int {
  "an II marker writes everything after it little-endian"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto written = s2s::stream_cast<tiff_header>(
      stream, make_header(std::array<u8, 2>{'I', 'I'}, 0xcafed00d));

    expect(eq(written.has_value(), true));
    expect(eq(buffer[0], u8{'I'}));
    expect(eq(buffer[1], u8{'I'}));
    expect(eq(buffer[2], u8{0x2a}));
    expect(eq(buffer[3], u8{0x00}));
    expect(eq(buffer[4], u8{0x0d}));
    expect(eq(buffer[5], u8{0xd0}));
    expect(eq(buffer[6], u8{0xfe}));
    expect(eq(buffer[7], u8{0xca}));
  };

  "an MM marker writes everything after it big-endian"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto written = s2s::stream_cast<tiff_header>(
      stream, make_header(std::array<u8, 2>{'M', 'M'}, 0xcafed00d));

    expect(eq(written.has_value(), true));
    expect(eq(buffer[0], u8{'M'}));
    expect(eq(buffer[1], u8{'M'}));
    expect(eq(buffer[2], u8{0x00}));
    expect(eq(buffer[3], u8{0x2a}));
    expect(eq(buffer[4], u8{0xca}));
    expect(eq(buffer[5], u8{0xfe}));
    expect(eq(buffer[6], u8{0xd0}));
    expect(eq(buffer[7], u8{0x0d}));
  };

  "what struct_cast reads, stream_cast writes back"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto written = s2s::stream_cast<tiff_header>(
      stream, make_header(std::array<u8, 2>{'M', 'M'}, 0xcafed00d));
    stream.rewind();
    auto res = s2s::struct_cast<tiff_header>(stream);

    expect(eq(written.has_value(), true));
    expect(eq(res.has_value(), true));
    expect(eq((*res)["magic"_f], u16{42}));
    expect(eq((*res)["ifd_offset"_f], 0xcafed00du));
  };

  // The criterion that the marker keeps its setter, stated as the thing a
  // caller would actually do: the same struct type and the same offset, one
  // assignment apart, and the bytes come out in the other order.
  "setting the marker chooses the output order"_test = [] constexpr {
    std::array<u8, 8> little{};
    std::array<u8, 8> big{};
    memstream<8> little_stream(little);
    memstream<8> big_stream(big);

    auto le = s2s::stream_cast<tiff_header>(
      little_stream, make_header(std::array<u8, 2>{'I', 'I'}, 0xcafed00d));
    auto be = s2s::stream_cast<tiff_header>(
      big_stream, make_header(std::array<u8, 2>{'M', 'M'}, 0xcafed00d));

    expect(eq(le.has_value(), true));
    expect(eq(be.has_value(), true));
    expect(eq(little[4], u8{0x0d}));
    expect(eq(big[4], u8{0xca}));
    expect(eq(little[7], u8{0xca}));
    expect(eq(big[7], u8{0x0d}));
  };

  // Resolution at depth is its own code path on write, so it gets its own case
  // rather than riding on the read side's nesting tests.
  "a nested announcement decides the order on write too"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);

    auto written =
      s2s::stream_cast<nested_once>(stream, make_nested(std::array<u8, 2>{'M', 'M'}));

    expect(eq(written.has_value(), true));
    expect(eq(buffer[2], u8{0x01}));
    expect(eq(buffer[3], u8{0x2c}));
    expect(eq(buffer[4], u8{0xca}));
    expect(eq(buffer[7], u8{0x0d}));
  };

  // The proof of the seeding decision: this struct is default-constructed and
  // nobody can assign its marker, so without field::value being seeded from the
  // eq constraint the resolution would read {0, 0}, match no case, and fail.
  "an eq-constrained marker writes from a default-constructed struct"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);
    always_big_endian obj{};
    obj["value"_f] = 0xcafed00d;

    auto written = s2s::stream_cast<always_big_endian>(stream, obj);

    expect(eq(written.has_value(), true));
    expect(eq(buffer[0], u8{'M'}));
    expect(eq(buffer[1], u8{'M'}));
    expect(eq(buffer[2], u8{0xca}));
    expect(eq(buffer[5], u8{0x0d}));
  };

  "a frozen marker reads back the order it wrote"_test = [] constexpr {
    std::array<u8, 8> buffer{};
    memstream<8> stream(buffer);
    always_big_endian obj{};
    obj["value"_f] = 0xcafed00d;

    auto written = s2s::stream_cast<always_big_endian>(stream, obj);
    stream.rewind();
    auto res = s2s::struct_cast<always_big_endian>(stream);

    expect(eq(written.has_value(), true));
    expect(eq(res.has_value(), true));
    expect(eq((*res)["value"_f], 0xcafed00du));
  };
}
