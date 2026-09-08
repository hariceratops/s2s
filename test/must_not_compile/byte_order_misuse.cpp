// The byte-order axis rejects what it cannot resolve.
// Design: dev/design/byte-order-is-data-not-a-template-parameter.md §10
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.
// With no CASE defined this file must build cleanly — that is the control
// build every case is checked against before being registered.
//
// NOTHING IS REGISTERED IN CMakeLists.txt AT SCAFFOLD TIME. add_rejected_case
// sets WILL_FAIL, so a case that fails for the wrong reason passes the harness
// while testing nothing — the trap 047 fell into on CASE 4 and that 048-052
// documented. Each case is registered by the slice that implements it, only
// after being confirmed to fail on its own diagnostic rather than on an
// unrelated arity error.
//
// Assert only that the build fails. gcc, clang and MSVC word concept
// diagnostics differently, so pinning message text would make this suite
// non-portable.
//
// CASE 1 (058): struct_cast on a schema with no announcement.
// CASE 2 (058): struct_cast_le on a self-announcing schema.
//               Both are registered with add_rejected_case_matching rather than
//               add_rejected_case: the diagnostic is our own static_assert
//               text, so the build has to fail *for that reason* rather than
//               merely fail.
// CASE 3 (056): an order-dependent u16 directly in the announcing record.
// CASE 4 (056): the same, nested one record deeper — proves the walk descends.
// CASE 5 (060): two announcements in disjoint subtrees — also proves the
//               census is cumulative rather than per-level.
// CASE 6 (057): an announcement behind a maybe — reached conditionally, so a
//               cast can finish without ever resolving. Design §5.2.
// CASE 10 (057): an announcement inside an array of records — reached
//               repeatedly, so the last element would silently win.
// CASE 7 (055): a match_field naming a field the announcing record lacks.
// CASE 8 (059): stream_cast on a schema with no announcement.
// CASE 9 (059): stream_cast_le on a self-announcing schema.
//               Registered with add_rejected_case_matching, like 1 and 2.

#include <array>
#include <bit>
#include <fstream>

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

#if CASE == 1 || CASE == 2 || CASE == 8 || CASE == 9
using entry_point_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using self_announcing =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", entry_point_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"magic", u32>
  >;

using fixed_order =
  s2s::struct_field_list<
    s2s::basic_field<"magic", u32>
  >;
#endif

#if CASE == 1
// Must NOT compile — struct_cast says the file decides its own byte order, and
// this schema declares nothing that could. Calling it here would be a claim
// about the format that is not true.
auto read_a_fixed_order_schema_with_struct_cast(std::ifstream& file) {
  return s2s::struct_cast<fixed_order>(file);
}
#endif

#if CASE == 2
// Must NOT compile — the other direction, and the one with teeth: the file
// carries its own order and the caller is naming a different one. There is
// deliberately no way to force a fixed order over a self-announcing schema.
auto read_a_self_announcing_schema_with_struct_cast_le(std::ifstream& file) {
  return s2s::struct_cast_le<self_announcing>(file);
}
#endif

#if CASE == 8
// Must NOT compile — the write-side mirror of CASE 1. stream_cast takes the
// order from the struct's marker, and this schema has none to take it from.
auto write_a_fixed_order_schema_with_stream_cast(std::ofstream& file, const fixed_order& obj) {
  return s2s::stream_cast<fixed_order>(file, obj);
}
#endif

#if CASE == 9
// Must NOT compile — the mirror of CASE 2, and the reason the write pair gets
// the same treatment as the read pair: this call would emit bytes in an order
// the marker sitting in the same struct contradicts.
auto write_a_self_announcing_schema_with_stream_cast_le(std::ofstream& file,
                                                        const self_announcing& obj) {
  return s2s::stream_cast_le<self_announcing>(file, obj);
}
#endif

#if CASE == 3 || CASE == 4
using order_guide =
  s2s::order_from<s2s::match_field<"marker">,
    s2s::order_switch<
      s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
      s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>;
#endif

#if CASE == 3
// Must NOT compile — a u16 in the announcing record is read before the record's
// own deduction runs, so it would be decoded at whatever the cell was seeded
// with. Rejected by announcing_record_check's order-agnostic assertion.
using order_dependent_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>,
    s2s::basic_field<"version", u16, 2_B>
  >;

using order_dependent_field_in_announcing_record =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_dependent_marker, order_guide>,
    s2s::basic_field<"magic", u32>
  >;
#endif

#if CASE == 4
// Must NOT compile — the same u16, one record deeper. This is the case that
// proves the walk descends rather than inspecting one flat pack; CASE 3 passes
// with a check that only looks at the announcing record's immediate fields.
using nested_order_dependent =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 2_B>
  >;

using deeply_order_dependent_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>,
    s2s::struct_field<"nested", nested_order_dependent>
  >;

using order_dependent_field_nested_deeper =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", deeply_order_dependent_marker, order_guide>,
    s2s::basic_field<"magic", u32>
  >;
#endif

#if CASE == 5
// Must NOT compile — two announcements in disjoint subtrees. Neither level
// below the top sees both, so this also proves the census is cumulative rather
// than per-level. Rejected as a limitation rather than as a conflict: a second
// announcement taking over from the first follows from the same
// stream-position rule, and is left undecided only until a real format settles
// what it should mean.
using disjoint_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using announcing_region =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", disjoint_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"magic", u32>
  >;

using two_announcements =
  s2s::struct_field_list<
    s2s::struct_field<"first", announcing_region>,
    s2s::struct_field<"second", announcing_region>
  >;
#endif

#if CASE == 6 || CASE == 10
using nested_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using announcing_header =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", nested_marker,
      s2s::order_from<s2s::match_field<"marker">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"magic", u32>
  >;
#endif

#if CASE == 6
// Must NOT compile — an announcement reached conditionally. The cast can finish
// without the header ever being read, and everything after it would then be
// decoded at the seed order. Rejected by the census's off-spine count.
using conditionally_announced =
  s2s::struct_field_list<
    s2s::basic_field<"present", u8, 1_B>,
    s2s::maybe<
      s2s::struct_field<"header", announcing_header>,
      s2s::parse_if<[](u8 present) { return present != 0; }, "present">
    >,
    s2s::basic_field<"payload", u32>
  >;
#endif

#if CASE == 10
// Must NOT compile — an announcement reached repeatedly. Each element would
// resolve an order and the last one would silently win.
using repeatedly_announced =
  s2s::struct_field_list<
    s2s::array_of_records<"headers", announcing_header, 3>,
    s2s::basic_field<"payload", u32>
  >;
#endif

#if CASE == 7
// Must NOT compile — the announcement names "markr", and the announcing record
// has "marker". Names resolve inside that record's own field table, so the
// misspelling is not a name from an enclosing scope either; it is nothing.
// Rejected by announcing_record_check's static_assert, which also reaches the
// caller as an unsatisfied constraint on create_struct_field_list.
using order_marker =
  s2s::struct_field_list<
    s2s::fixed_array_field<"marker", u8, 2>
  >;

using misspelled_announcement_name =
  s2s::struct_field_list<
    s2s::announces_byte_order<"byte_order", order_marker,
      s2s::order_from<s2s::match_field<"markr">,
        s2s::order_switch<
          s2s::order_case<std::array<u8, 2>{'I', 'I'}, std::endian::little>,
          s2s::order_case<std::array<u8, 2>{'M', 'M'}, std::endian::big>>>>,
    s2s::basic_field<"magic", u16, 2_B>
  >;
#endif

auto main() -> int {}
