// Options declared on a union alternative, read path, compile-time tier.
//
// Its own pair rather than additions to union_read*, because the tree is
// organised by what a file verifies: this checks an *option on an alternative*,
// not the variance construct. Same reasoning that gave allocation_bound_read*
// its own pair in 047.
//
// Scaffolded ahead of 050/051. The baseline below is real and passes today —
// it is the unconstrained, unbounded alternative every TODO case contrasts
// against, so a regression in plain variance reading shows up here rather than
// being mistaken for an option bug.
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
  "an alternative with no options reads"_test = [] constexpr {
    std::array<u8, 12> buffer{0x01, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00,
                              0x2a, 0x00, 0x00, 0x00};
    memstream<12> stream(buffer);

    auto res = s2s::struct_cast_le<plain>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<u32>((*res)["body"_f]), 42u));
  };

  "a container alternative with no options reads"_test = [] constexpr {
    std::array<u8, 11> buffer{0x02, 0x00, 0x00, 0x00,
                              0x03, 0x00, 0x00, 0x00,
                              0xaa, 0xbb, 0xcc};
    memstream<11> stream(buffer);

    auto res = s2s::struct_cast_le<plain>(stream);

    expect(eq(res.has_value(), true));
    expect(eq(std::get<std::vector<u8>>((*res)["body"_f]).size(), std::size_t{3}));
  };

  // TODO(050): a constrained alternative whose payload satisfies its constraint
  // reads normally.
  //
  // TODO(050): the same alternative whose payload violates it is rejected with
  // error_reason::validation_failure and failed_at == "body" — the union's id,
  // not the alternative's, because to_field_choices gives every choice the
  // union's id. union_choice_pipeline_ct asserts that at the type level.
  //
  // TODO(050): NON-VACUITY, READ SIDE. This case must fail if the constraint
  // check in read_variant_impl is deleted, and must be a *separate test* from
  // the write-side one in union_alternative_options_write_ct.cpp. One test
  // spanning both paths passes with either call site missing, which is the
  // whole failure this pair exists to rule out. Verify by deleting the check
  // locally and confirming exactly one test goes red.
  //
  // TODO(050): a constraint entry and a size entry on the same tag, spelled in
  // both orders, produce the same result — order-independence is the claim.
  //
  // TODO(051): below-bound accepted, at-bound accepted (inclusive), over-bound
  // rejected with error_reason::excessive_length, for as_vec, as_string and
  // as_vec_of_records alternatives.
  //
  // TODO(051): a bound that is not a whole multiple of sizeof(element) — the
  // gate is phrased as an integer division, so the rounding direction is
  // observable and has to be pinned.
  //
  // TODO(051): a declared alternative bound is not clamped by
  // S2S_DEFAULT_MAX_BYTES in either direction. Needs its own translation unit
  // to vary the macro; allocation_bound_default_override_ct.cpp and
  // allocation_bound_disabled_ct.cpp are the models.
}
