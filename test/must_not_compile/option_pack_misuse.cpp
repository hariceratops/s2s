// The trailing option pack rejects what it cannot classify.
//
// 045 makes a field's trailing parameters an order-independent pack of auto
// NTTPs, each classified as a size or a constraint by a named concept. Two
// things must be rejected, and they are rejected by different mechanisms:
// an unrecognised entry fails the per-element placeholder constraint, while a
// duplicate of a kind already supplied fails the design's one sanctioned
// static_assert.
//
// Assert only that the build fails. The design measured gcc 14's wording for
// the per-element form; clang and MSVC word concept diagnostics differently,
// so pinning message text would make this suite non-portable.
//
// Built with -DCASE=<n>; every case here is expected to fail compilation.

#include "../../single_header/s2s.hpp"

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

#if CASE == 1
// TODO(045): must NOT compile — 42 is neither a size nor a constraint. The
// diagnostic should isolate this entry, name its type, and name both things it
// could have been ("no operand of the disjunction is satisfied"), which is what
// the per-element placeholder constraint buys over a fold in a requires-clause.
using unrecognised_entry =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 42>
  >;
#endif

#if CASE == 2
// TODO(045): must NOT compile — two sizes in one pack. Order-independence means
// the classifier scans rather than reads positionally, so nothing about the
// grammar stops a second size being written; the duplicate-option count is what
// rejects it.
using duplicate_size =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 2_B, 2_B>
  >;
#endif

#if CASE == 3
// Must NOT compile — a bound is meaningless on a field whose extent
// is a template parameter, since nothing a stream says can change how much it
// allocates. It must fail the per-element placeholder constraint the same way
// an unrecognised entry does, not be silently ignored: silence would let a
// schema author believe they had bounded something they had not.
using bound_on_a_fixed_size_field =
  s2s::struct_field_list<
    s2s::basic_field<"version", u16, 2_B, s2s::max_bytes<4096>>
  >;
#endif

#if CASE == 4
// Must NOT compile — two bounds in one pack, caught by the same
// duplicate-count assertion pack_options already applies to sizes and
// constraints. Without it the scan would silently take the first.
using duplicate_bound =
  s2s::struct_field_list<
    s2s::basic_field<"n", u32, 4_B>,
    s2s::vec_field<"payload", u8, s2s::len_from_field<"n">,
                   s2s::max_bytes<4096>, s2s::max_bytes<8192>>
  >;
#endif

// ---------------------------------------------------------------------------
// Cases 5-10 are scaffolded for the union option pack (049-052) and are
// DELIBERATELY NOT REGISTERED in CMakeLists.txt yet.
//
// add_rejected_case sets WILL_FAIL, so a registered case that fails for the
// wrong reason — an undeclared type, a name that does not exist yet — passes
// the harness while testing nothing. Every case below currently fails because
// the tag does not take a pack at all, which is not the diagnostic it is meant
// to prove. Register each one in the slice that implements it, and check it
// against a no-CASE control build first: the control must compile cleanly, and
// the case must fail on its own diagnostic.
// ---------------------------------------------------------------------------

#if CASE == 5
// TODO(049): must NOT compile — two size entries in one tag's pack. The tag
// reuses pack_options, so this is the same duplicate-count assertion a
// descriptor already applies, reached through a different spelling.
using duplicate_size_on_tag =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_trivial<u32, 4_B, 2_B>>
      >>>
  >;
#endif

#if CASE == 6
// TODO(049): must NOT compile — as_string with an empty pack. This spelling is
// newly *legal grammar* once the size joins the pack (it was an arity error
// before), so it becomes reachable and has to be rejected on its merits: with
// no size entry the resolved size is byte_count{sizeof(std::string)}, which is
// not variable_size_like. It is the one spelling the change admits into the
// grammar, which is why it earns a case of its own.
using unsized_as_string =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_string<>>
      >>>
  >;
#endif

#if CASE == 7
// TODO(049): must NOT compile — a size larger than the underlying type. This is
// 049's "the relocated constraint must still constrain" criterion: as_trivial's
// requires-clause moves off a named parameter onto the resolved pack size, and
// a relocation that quietly stops constraining is the failure mode.
using oversized_as_trivial =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_trivial<u16, 4_B>>
      >>>
  >;
#endif

#if CASE == 8
// TODO(050): must NOT compile — two constraint entries in one tag's pack.
using duplicate_constraint_on_tag =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_trivial<u32, 4_B,
                                              s2s::gt{1u}, s2s::lt{99u}>>
      >>>
  >;
#endif

#if CASE == 9
// TODO(051): must NOT compile — a bound on an alternative with no wire-driven
// allocation. Same promise as CASE 3 makes for descriptors, reached through the
// tag: as_struct's extent comes from its schema, not from anything a stream
// says.
using inner_for_bound =
  s2s::struct_field_list<
    s2s::basic_field<"x", u32, 4_B>
  >;

using bound_on_a_sizeless_tag =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_struct<inner_for_bound, s2s::max_bytes<4096>>>
      >>>
  >;
#endif

#if CASE == 10
// TODO(052): must NOT compile — a size entry on variance itself. A union's own
// size is size_dont_care and it drives no allocation of its own, so its pack
// admits a constraint and nothing else.
using size_on_variance =
  s2s::struct_field_list<
    s2s::basic_field<"tag", u32, 4_B>,
    s2s::variance<"body", s2s::type<s2s::match_field<"tag">,
      s2s::type_switch<
        s2s::match_case<0x01, s2s::as_trivial<u32, 4_B>>
      >>, 4_B>
  >;
#endif

auto main() -> int {
  return 0;
}
