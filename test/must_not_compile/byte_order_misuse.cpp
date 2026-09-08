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

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

auto main() -> int {}
