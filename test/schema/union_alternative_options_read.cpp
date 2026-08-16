#include <cstdint>
#include <variant>
#include <vector>
#include <gtest/gtest.h>
#include "../../single_header/s2s.hpp"
#include "../utils/s2s_test_utils.hpp"


using namespace s2s_literals;

// The run-time half of union_alternative_options_read_ct.cpp. The compile-time
// file carries the matrix against a constexpr_memstream; this exists because
// the path has to be live in a non-constexpr build too, per the project's
// convention that stream-touching code needs both forms.
//
// Scaffolded ahead of 050/051; the baseline below is real and passes today.

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

auto stream_with(u32 tag, u32 n, const std::vector<u8>& payload) -> std::stringstream {
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  stream.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
  stream.write(reinterpret_cast<const char*>(&n), sizeof(n));
  stream.write(reinterpret_cast<const char*>(payload.data()),
               static_cast<std::streamsize>(payload.size()));
  return stream;
}
} /* namespace */

TEST(UnionAlternativeOptions, ReadsAnAlternativeCarryingNoOptions) {
  auto stream = stream_with(0x01, 0, {0x2a, 0x00, 0x00, 0x00});

  auto result = s2s::struct_cast_le<plain>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<u32>((*result)["body"_f]), 42u);
}

TEST(UnionAlternativeOptions, ReadsAContainerAlternativeCarryingNoOptions) {
  auto stream = stream_with(0x02, 3, {0xaa, 0xbb, 0xcc});

  auto result = s2s::struct_cast_le<plain>(stream);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(std::get<std::vector<u8>>((*result)["body"_f]).size(), 3u);
}

// TODO(050): a constrained alternative accepted, and rejected with
// validation_failure / failed_at == "body".
//
// TODO(050): non-vacuity, read side — must fail if read_variant_impl's check is
// deleted, and must stay separate from the write-side case.
//
// TODO(051): below-bound / at-bound / over-bound for as_vec, as_string and
// as_vec_of_records, asserting excessive_length on the last.
//
// TODO(051): a bound that is not a whole multiple of sizeof(element).
