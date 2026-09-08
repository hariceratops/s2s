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
// CASE 3 (056): an order-dependent u16 directly in the announcing record.
// CASE 4 (056): the same, nested one record deeper — proves the walk descends.
// CASE 5 (060): two announcements in disjoint subtrees — also proves the
//               census is cumulative rather than per-level.
// CASE 6 (057): an off-spine announcement (behind a maybe, or inside an array)
//               — design §5.2.
// CASE 7 (055): a match_field naming a field the announcing record lacks.
// CASE 8 (059): stream_cast on a schema with no announcement.
// CASE 9 (059): stream_cast_le on a self-announcing schema.

#include <array>
#include <bit>

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

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
