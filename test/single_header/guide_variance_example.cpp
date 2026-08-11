// Backs the variance/type-deduction example in UserGuide.md. The region between
// docs-begin and docs-end is compared against that fenced block by the
// doc_examples_match test, so edit them together or the check fails.
// docs-begin
#include "s2s.hpp"

#include <fstream>
#include <variant>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;
using i32 = int;

// Selection by discriminant. A tag-length-value record carries the tag on the
// wire, and it says how the value should be read.
using tlv_record =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u8, s2s::field_size<s2s::fixed<1>>>,
    s2s::variance<
      "value",
      s2s::type<
        s2s::match_field<"tag">,
        s2s::type_switch<
          s2s::match_case<1, s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>>,
          s2s::match_case<2, s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>>
        >
      >
    >
  >;

// Selection by predicate. Nothing on the wire names the payload shape; it
// follows from a length already read, the way formats inline small values and
// reference large ones.
constexpr auto fits_inline = [](auto length) { return length <= 4u; };
constexpr auto needs_reference = [](auto length) { return length > 4u; };

using extent_record =
  s2s::struct_field_list<
    s2s::basic_field<"length", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::variance<
      "payload",
      s2s::type<
        s2s::type_if_else<
          s2s::branch<
            s2s::predicate<fits_inline, s2s::with_fields<"length">>,
            s2s::as_trivial<u32, s2s::field_size<s2s::fixed<4>>>
          >,
          s2s::branch<
            s2s::predicate<needs_reference, s2s::with_fields<"length">>,
            s2s::as_trivial<i32, s2s::field_size<s2s::fixed<4>>>
          >
        >
      >
    >
  >;

auto main() -> int {
  // "tag" is never assigned: it is derived from the alternative held.
  tlv_record record{};
  record["value"_f] = i32{-40};

  std::fstream tlv("tlv_record.bin",
                   std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if(!tlv)
    return 1;
  if(const auto written = s2s::struct_write_be<tlv_record>(tlv, record); !written)
    return 1;
  tlv.seekg(0);
  const auto tlv_back = s2s::struct_cast_be<tlv_record>(tlv);
  if(!tlv_back
     || (*tlv_back)["tag"_f] != 2
     || std::get<i32>((*tlv_back)["value"_f]) != -40)
    return 1;

  // "length" stays assignable: a predicate has no inverse, so it is verified
  // against the alternative held rather than derived from it.
  extent_record extent{};
  extent["length"_f] = 4u;
  extent["payload"_f] = u32{0xfeedface};

  std::fstream ext("extent_record.bin",
                   std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  if(!ext)
    return 1;
  if(const auto written = s2s::struct_write_be<extent_record>(ext, extent); !written)
    return 1;
  ext.seekg(0);
  const auto ext_back = s2s::struct_cast_be<extent_record>(ext);
  if(!ext_back || std::get<u32>((*ext_back)["payload"_f]) != 0xfeedface)
    return 1;

  // The held alternative contradicts the branch the predicates select.
  extent_record inconsistent{};
  inconsistent["length"_f] = 64u;
  inconsistent["payload"_f] = u32{1};
  std::fstream discard("extent_bad.bin",
                       std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  const auto rejected = s2s::struct_write_be<extent_record>(discard, inconsistent);

  return !rejected
      && rejected.error().failure_reason == s2s::error_reason::validation_failure
        ? 0 : 1;
}
// docs-end
