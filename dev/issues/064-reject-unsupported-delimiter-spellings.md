# [feat] reject the delimiter spellings the form does not support

With the read surface complete at 063, the spellings `until<>` must not accept
need rejections that are deliberate rather than incidental. Held as its own
slice on 058's precedent.

**This slice is one line of production code and four test targets.** The design
(§1.4, §10 F3) changed its shape from how it was first written. The original
framing treated every rejection as a property to *verify* — "the widened clauses
must not have widened so far that these compile". That is wrong for one case and
right for the rest:

- `basic_field` takes `field_option_like`, which includes `size_option_like`. The
  arm 062 adds to that concept is precisely what lets `until<>` reach
  `basic_field` and proceed into `field_fits_to_underlying_type`, whose body
  instantiates `deduce_field_size` — an incomplete type inside an atomic
  constraint. Whether that is a substitution failure or a hard error is a
  question about the immediate context not worth relying on, and either
  diagnostic is about arithmetic rather than about the rule. So `basic_field`
  gains `fixed_size_like<size_type_of<size_of_pack<T, opts...>>>` ahead of the
  fits-check. Conjunction short-circuits, so the fits-check is never substituted
  for a non-fixed size. This rejects nothing that compiles today: a
  `len_from_field` on a `basic_field` already fails the same check, only later
  and less legibly.
- `fixed_array_field`, `c_str_field`, `fixed_string_field`, `c_arr_field`, the
  `magic_*` family, `struct_field`, `variance` and `announces_byte_order` take
  **constraint options only**. `until<>` on any of them is an unrecognised pack
  entry today and stays one. These need no code at all — only test entries.
- The multi-byte delimiter is rejected by `delimiter_value_like`, which 062
  introduces. No code here either.

**On diagnostics.** An earlier version of this issue asked that "each diagnostic
names what was wrong, not merely that a constraint was unsatisfied". That
contradicts the repo's own policy: `must_not_compile/byte_order_misuse.cpp`'s
header states that concept wording differs across gcc, clang and MSVC, so
pinning message text is not portable, and `field_options.hpp` reserves
`static_assert` for what a concept cannot express (counting). Every rejection
here is expressible as a concept, so **the concept names are the diagnostics** —
`delimiter_value_like`, `fixed_size_like` — and each case registers with
`add_rejected_case`, not `add_rejected_case_matching`. Changing this to
sentence-shaped messages would be a deliberate departure from that policy, not a
drive-by.

Design: `dev/design/the-size-axis-cannot-say-read-until-a-byte.md`, §1.4 and §8.
Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- `basic_field` gains `fixed_size_like` on its size ahead of
  `field_fits_to_underlying_type`, and `until<>` on a `basic_field` fails to
  compile as a result.
- That clause rejects nothing that compiles today; the existing schema and
  traits suites pass unmodified.
- `until<u16{0x0d0a}>` fails to compile on `delimiter_value_like`.
- `until<>` on `fixed_array_field` fails to compile as an unrecognised pack
  entry, with no new code.
- `until<>` on `c_str_field` fails to compile the same way, with no new code.
- Each rejection is its own `test/must_not_compile/` target, per that directory's
  one-target-per-program convention, registered with `add_rejected_case`.
- No case pins compiler diagnostic text.
- **Each case is checked against a no-`CASE` control build and confirmed to fail
  on its own diagnostic before registration** — a `must_not_compile` entry that
  fails for an unrelated reason passes just as green, which is the trap 047 fell
  into.
- The wide-element rejection for `vec_field` is 063's, not this slice's.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
