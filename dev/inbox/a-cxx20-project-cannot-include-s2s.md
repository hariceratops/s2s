# A C++20 project cannot include s2s

Brief, from a design conversation on 2026-11-07. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

s2s requires C++23: `CMakeLists.txt:10` sets it, and the README (line 61) and
`docs/index.md` (line 47) state it. A translation unit compiled as C++20 cannot
include `single_header/s2s.hpp` at all.

The developer wants a library tiered by language version, not a port down to
C++20: "we support both current and future version with preprocessor switches
so that projects locked to certain versions can use the version of s2s which is
applicable".

- C++20 becomes the supported floor.
- Features that need C++23 switch on above it.
- The annotated-struct front end switches on in a C++26 tier.
- `struct_field_list` exists in every tier, and both front ends feed one backend
  (see `a-schema-cannot-be-a-plain-struct.md`).

**This is not v1 work.** v1, due January 2027, is s2s feature-complete in C++23:

- packed bitfields;
- the lazy-records generator;
- benchmarks against hand-written reads and writes, plus the copy removal they
  call for (`a-parsed-record-is-copied-on-its-way-out.md`);
- the naming sweep and dead code removal.

The developer's words: "the complete version compatibility in both version is
after we are feature complete in c++23 since that was the initial pitch". The
C++20 tier is the first job after v1. January is the code deadline. The Meeting
C++ talk comes later, usually March to June 2027 or after, and its content is
undecided.

## Considerations the discussion surfaced

**What is settled.**

- **Detection:** C++ feature-test macros.
- **The tier is strictly 20 or 23 from s2s's side, and abstracted from the
  user's** ("its 20 or 23 from s2s perspective from user perspective it should
  remain abstracted"). A user flag such as `S2S_20` cannot select s2s's own
  expected in a C++23 build.
- **One single header in v1.** The amalgam is split per version only "in case
  build time explosion".
- **Expected:** `std::expected` from C++23 up, "this way a bug in our expected
  only creeps < cpp23". Below that, s2s's own expected, which "must be
  identical to c++23".
- **Tests:** the complete suite runs at every tier. A test for a feature that
  exists only in some tiers states its floor.
- **Generator:** in the long run, a hand-written generator for the C++20 tier and
  `std::generator` for C++23 and up, chosen by tier. v1 starts with
  `std::generator`. The developer codes it by hand, pairing with an agent, to
  learn coroutines, and decides the details there.
- **Docs:** whether they can be tier-agnostic is left to the architect.

`AGENTS.md` carries a rule on the preprocessor. It records only build-wide
choices made once: the language tier, and whether the environment is hosted.
Code paths are selected with `if constexpr`, `if consteval` and concepts, not
with `#if` inside a function, "even where it makes `scripts/amalgam.py` harder
to write".

**What the C++20 tier loses or replaces in the library: two features.** A grep of
`include/` for every C++23 feature turns up:

- `std::expected`/`std::unexpected`: 69 occurrences in 16 headers, all of which
  include `<expected>`;
- `std::byteswap`: two calls inside one function, `byteswapped()` at
  `stream/byte_order.hpp:29,36`. A hand-rolled byteswap sits commented out in
  `lib/memory/bit.hpp`, and `read_impl.hpp:140` carries a todo for exactly this.

Nothing else turns up:

- no deducing `this`, no static `operator()`, no `if consteval`;
- no `std::unreachable`, and no `std::to_underlying` (the three hits are the
  concept `field_fits_to_underlying_type`);
- no monadic `std::optional`, no multidimensional subscript, and no `<print>` in
  `include/`.

Everything else the library relies on is already C++20: class-type NTTPs, concepts,
constexpr `std::vector`/`std::string`, `std::bit_cast`, `std::span`,
`std::to_array`, and lambdas in unevaluated operands (`mp.hpp:52`).

**The C++20 tier also gains a generator, written from scratch.** v1's generator
is `std::generator`, which is C++23. Under the settled long-term plan, the C++20
tier gets a hand-written one over `<coroutine>`. That makes it new code in the
port, not a port of v1's code.

The two generators have to present the same interface to the caller, as the
expected does, since the user's side "should remain abstracted". Whether
`std::generator` is available at all varies by library, and isn't verified:
libc++ may lack `<generator>`, and the MSVC 19.39 floor may predate it.

**Grep cannot see the language relaxations.** C++23 loosened constexpr in three
ways that leave no keyword: P2647, P2448 and P2280. The only reliable census is
to compile the suite at `-std=c++20` on all three compilers. That takes an
afternoon, and its result replaces every estimate here.

**The "workarounds for cpp20" already in s2s are few, and none is behind a
switch.** The developer suggested an `S2S_20` flag because "we are having
multiple other workarounds for cpp20 in s2s". A search of `include/` finds
three:

- **`dependent_false`** (`lib/s2s_traits/type_traits.hpp:277-281`, used at
  `read_impl.hpp:183`, `write_impl.hpp:97`, `announcing_record.hpp:131` and
  `sentinel_terminator.hpp:84`). It stands in for `static_assert(false, ...)` in
  a discarded branch, which P2593 permits from C++23. It already works in C++20.
- **`terminator_field_check`** (`sentinel_terminator.hpp:89-91`). Its comment
  dates formatted `static_assert` messages to C++23. I believe they are P2741,
  which is C++26.
- **The commented-out byteswap** in `lib/memory/bit.hpp`, which is dead.

All of these are unconditional. If the developer means something else, it wasn't
found.

**With the tier abstracted from users, it is unclear what a user flag would
do.** The settled answer rules out one job for `S2S_20`: it cannot choose the
expected in a C++23 build. And if "from user perspective it should remain
abstracted", users may not choose a tier at all, with feature-test macros
deciding. That leaves no obvious job for a user flag, except perhaps switching
workarounds like those above. Whether there is a user flag at all is open.

"20 or 23" also leaves the C++26 annotated front end unplaced. It could be a
third tier, or a front-end switch on top of the C++23 tier.

**The preprocessor rule meets two-phase lookup.** `if constexpr` can discard a
branch, but a template's non-dependent names are still looked up when the
template is defined. So a discarded branch that names `std::expected`,
`std::byteswap` or `std::generator` is still ill-formed in a C++20 translation
unit. `^^T` and `template for` don't even lex below C++26.

Choosing between declarations that exist in some tiers and not others therefore
takes a `#if` at namespace scope around each declaration:

- the expected alias;
- the byteswap implementation;
- which generator is defined;
- the whole annotated front end.

The rule allows a build-wide choice and forbids `#if` inside a function. It
should say outright that this namespace-scope form is allowed.

**The rule names a C++23 tool.** `if consteval` is C++23; the C++20 tier has only
`std::is_constant_evaluated()`. s2s uses neither today. It separates the
constexpr and runtime stream paths with concepts (`identified_as_constexpr_stream`,
used at `read_impl.hpp:82` and `write_impl.hpp:39`), and concepts work at every
tier.

**"20 or 23" is one value; feature-test macros are many.** `__cpp_lib_expected`,
`__cpp_lib_byteswap`, `__cpp_lib_generator` and `__cpp_impl_reflection` each
report one capability, and they don't always move together. Two cases test the
"20 or 23" settlement:

- a C++23 build on a library without `<generator>`;
- a C++26 build without `-freflection`.

**Reflection is where feature-test macros clearly win.** Status checked
2026-11-07:

- **GCC 16** (16.1 April 2026, 16.2 August 2026): P2996R13 under `-std=c++26
  -freflection`; P3394R4 without `apply_result`, `is_applicable_type` or
  `is_nothrow_applicable_type`; P1306R5 and P3096R12.
- **Upstream Clang:** P2996, P3394 and P3491 "No"; P1306 "Clang 23 (Partial)".
  Only the Bloomberg clang-p2996 fork has them, behind `-freflection-latest`.
- **MSVC:** nothing public.

`__cplusplus` cannot tell whether `-freflection` was passed. MSVC also reports
`199711L` unless `/Zc:__cplusplus` is set.

**The single header cannot carry a conditional standard include today.**
`scripts/amalgam.py` matches every `#include <...>` line wherever it appears
(line 8). It strips the line in place (line 120) and writes the full set at the
top, sorted and unconditional (lines 110-111). A tier-guarded `#include
<expected>`, `<generator>` or `<meta>` therefore comes out unconditional:

- On libstdc++, `<expected>` is probably harmless, since its contents are
  guarded.
- I believe MSVC's STL warns, which breaks `test/shipped_header/`'s
  warning-clean check.
- `<meta>` on a compiler without it is a hard error.

The one single header needs `amalgam.py` to preserve conditional blocks and
still regenerate byte-identically. Nothing yet measures "build time explosion".

**Mixed-tier ODR is a concrete risk.** A program can link a C++20 translation
unit and a C++23 one that both include s2s.

1. `struct_cast_impl<S, stream>::operator()` (`cast/struct_cast_impl.hpp:21`)
   is a non-template member of a class template.
2. Its return type is `std::expected<S, cast_error>` at C++23 and
   `s2s::expected<S, cast_error>` at C++20.
3. On Itanium-ABI compilers, a non-template member's mangled name does not
   include its return type.
4. Both translation units therefore emit the same inline symbol with different
   bodies, and the linker silently keeps one.

A C++23 caller can then run the C++20 body, which is the leak answer 4 was
chosen to prevent. "Strictly 20 or 23" settles what s2s sees inside one
translation unit, but not what happens when two tiers meet at link time. An
inline namespace per tier gives them different names.

**"Identical to C++23" is a large, precise surface.** It covers:

- the member interface: `and_then`, `or_else`, `transform`, `transform_error`,
  `value_or`, `value`, `error`, comparison, `swap`, and the `void`
  specialisation;
- `std::unexpected`, `unexpect_t`, and `bad_expected_access`.

It must be as constexpr as the standard one, because the ut tier runs at compile
time with values holding `std::vector`/`std::string`. That needs union storage, a
conditionally trivial destructor and `std::construct_at`, all of which exist in
C++20.

**`value()` without exceptions is flagged for discussion.** `value()` throws
`bad_expected_access`, so an identical interface throws too. That collides with
an exception-free or freestanding build (`a-firmware-build-cannot-use-s2s.md`).
The developer has flagged this as an item to discuss.

**One suite at every tier changes the tests themselves.**

- **The 312 ut lambdas.** They are written `[] constexpr { ... }` across 40
  files. Omitting `()` before a specifier is C++23 (P1102), so they have to
  become `[]() constexpr {`.
- **`<print>`.** `examples/hello_s2s.cpp` and
  `test/doc_examples/readme_read_example.cpp` use it, and the latter is bound to
  the README.
- **Frameworks.** GoogleTest v1.17.0 needs C++17. qlibs/ut v2.1.6 is C++20 as
  far as I know; check this.
- **Tier floors.** Settled: tier-bound tests state their floor. Under the
  `AGENTS.md` rule, that belongs in CMake, which chooses sources per tier, not in
  `#if` inside test files. The v1 generator's tests are the first to need one,
  with a C++23 floor until the hand-written generator exists.
- **The build matrix.** Three builds per ut source, times the tiers, times three
  compilers. `.github/workflows/` holds only `docs.yml`.

**Bitfields are the same in every tier.** They use shift-and-mask over an integer,
with byte order through `byteswapped()`.

**The gcc floor does not move.** `CMakeLists.txt:17-21` rejects gcc below 14 for
the constexpr path, whatever `-std` says.

**`AGENTS.md` changes when the tier lands.** "Target is hosted C++23" becomes a
tier statement. The `std::expected` override's first justification, "the C++23
baseline", becomes "from the C++23 tier up, and an identical s2s type below".

**What this means for January.** Nothing in this brief ships in v1. v1's
refactoring is bounded to benchmarks, tuning, the naming sweep and dead code
removal. Value-based changes follow the v1 rule recorded in the schema brief:
new data code, code already open for v1 work, and code a benchmark shows is the
problem. That rule produces plain `constexpr` functions over values, which are
already C++20, so it adds nothing to port.

v1 can still make the port cheaper:

- the cleanup's inventory of what is tier-bound;
- no new C++23-only spellings in new tests.

The afternoon `-std=c++20` compile census would also fit.

## Directions rejected

**A port that lowers the floor for everyone.** The developer's tiering rules it
out.

**A user flag that selects s2s's own expected in a C++23 build.** "Its 20 or 23
from s2s perspective". The expected follows the real standard.

**`#if` inside functions to select tier-specific code paths.** `AGENTS.md`
forbids it.

**Int error codes in the C++20 tier.** The override rejects them for the whole
project.

**`#if` at the 69 expected sites.** The choice lives in one place, and
`cast_error.hpp` already defines `rw_result` and `cast_result`.

**A third-party expected.** `dev/design/struct-to-stream-write.md:684-689` rules
out external dependencies for the single header.

## Questions still open

- Is there a user-facing flag at all, now that the tier is abstracted from users?
  If so, what does it select? And what are the "workarounds for cpp20" the
  developer has in mind?
- Where does the C++26 annotated front end sit: a third tier, or a front-end
  switch on top of 23?
- How does "20 or 23" handle a C++23 build without `<generator>`, and a C++26
  build without `-freflection`?
- Should the `AGENTS.md` rule state that a namespace-scope `#if` choosing which
  declarations exist is allowed? What does the rule say instead of `if
  consteval` for the C++20 tier?
- Is mixing tiers in one program supported? If so, is an inline namespace per
  tier the guard?
- How does `amalgam.py` preserve conditional includes and declaration blocks
  while staying byte-identical? What measurement counts as "build time
  explosion"?
- What does `value()` do in a build without exceptions? Flagged by the developer
  for discussion.
- Do the hand-written and standard generators present an identical interface,
  the way the two expecteds must?
- Can the docs be tier-agnostic? Left to the architect.
- Which compilers and tiers does CI cover?
