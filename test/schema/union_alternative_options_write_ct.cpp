// Options declared on a union alternative, write path, compile-time tier.
// Companion to union_alternative_options_read_ct.cpp; see its header for why
// this is its own pair rather than an addition to union_write*.
//
// The write side is a separate file, not a separate test in the read file, for
// the reason 050 turns on: the per-alternative constraint needs an explicit
// check at write_variant_impl *and* one at read_variant_impl, and a single test
// exercising a round-trip would stay green with either one missing.
//
// No `using namespace ut;` — ut exports eq/neq/lt/gt/le/ge, which collide with
// the s2s constraints of the same names. Test lambdas must not capture: ut
// skips a capturing lambda at compile time silently.

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

auto main() -> int {
  // Only "body" is assigned: "tag" is the discriminant and "n" the length
  // target, both derived from the held alternative rather than stored, and 043
  // made them non-assignable so a schema author cannot contradict the
  // derivation.
  "an alternative with no options writes"_test = [] constexpr {
    plain obj{};
    obj["body"_f] = 42u;

    std::array<u8, 12> buffer{};
    memstream<12> stream(buffer);

    auto res = s2s::stream_cast_le<plain>(stream, obj);

    expect(eq(res.has_value(), true));
    expect(eq(buffer[0], u8{0x01}));
    expect(eq(buffer[8], u8{0x2a}));
  };

  // TODO(050): a constrained alternative whose payload satisfies its constraint
  // writes normally.
  //
  // TODO(050): the same alternative whose payload violates it is rejected with
  // error_reason::validation_failure, before any bytes reach the stream.
  //
  // TODO(050): NON-VACUITY, WRITE SIDE. This case must fail if the constraint
  // check in write_variant_impl is deleted, and must stay independent of the
  // read-side case in union_alternative_options_read_ct.cpp. Verify by deleting
  // the check locally and confirming exactly one test goes red.
  //
  // TODO(052): a union-level constraint and a per-alternative constraint both
  // declared, each firing independently — 052 names this the interaction most
  // likely to be got wrong. Lives here rather than in a 052-specific file
  // because it is precisely the *combination* that is under test.
  //
  // Note for 051: there is deliberately no write-side bound case. A bound
  // guards an allocation driven by a wire length; the write path serialises a
  // container the caller already owns, so there is nothing untrusted to bound.
  // If a write-side bound case ever seems necessary, the spec's Non-Goals
  // ("write path untouched") is the thing to re-read first.
}
