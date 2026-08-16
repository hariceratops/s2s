#include <cstdint>
#include <variant>
#include <vector>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of union_alternative_options_write_ct.cpp. See that file
// for why the write cases live apart from the read ones rather than sharing a
// round-trip: 050's two constraint call sites have to be shown to fail
// independently.

namespace {
using plain =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::basic_field<"n", u32, 4_B>,
    s2s::variance<
      "body",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>,
          s2s::match_case<0x02, s2s::as_vec<u8, s2s::len_from_field<"n">>>
        >
      >
    >
  >;
} /* namespace */

TEST(UnionAlternativeOptionsWrite, WritesAnAlternativeCarryingNoOptions) {
  // Only "body" is assigned: "tag" is the discriminant and "n" the length
  // target, both derived from the held alternative rather than stored, and 043
  // made them non-assignable so a schema author cannot contradict the
  // derivation.
  plain obj{};
  obj["body"_f] = 42u;

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  ASSERT_TRUE(s2s::stream_cast_le<plain>(stream, obj).has_value());
  EXPECT_EQ(stream.str(), std::string("\x01\x00\x00\x00\x00\x00\x00\x00\x2a\x00\x00\x00", 12));
}

// TODO(050): a constrained alternative accepted on write, and rejected with
// validation_failure before any bytes reach the stream.
//
// TODO(050): non-vacuity, write side — must fail if write_variant_impl's check
// is deleted, and must stay separate from the read-side case.
//
// TODO(052): a union-level constraint and a per-alternative constraint both
// declared, each firing independently.
//
// No write-side bound cases: a bound guards an allocation driven by a wire
// length, and the write path serialises a container the caller already owns.
