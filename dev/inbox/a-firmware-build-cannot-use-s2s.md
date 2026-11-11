# A firmware build cannot use s2s

Brief, from a design conversation on 2026-11-07. Not a PRD — the open questions
at the bottom are the interview's agenda.

## The goal

The formats s2s describes are most at home on devices: the IPv4, TCP and DNS
headers, and the rest of the packed-byte brief's list. `AGENTS.md` calls s2s
"usable on embedded targets". In practice it is not:

- every entry point includes `<iostream>`;
- three descriptors are hard-wired to `std::vector` and `std::string`;
- every read entry point returns `std::expected`.

The developer's request is "supporting non stl constructs like vectors expected
and so on + freestanding".

**The target is the C++ standard's freestanding subset.** The developer calls
this "much easier" than a full toolchain with no heap and no exceptions. The goal
has two parts: let a schema read into containers the caller chooses, and let s2s
build in a freestanding implementation.

**This is post-v1.** v1, due January 2027, is s2s feature-complete in C++23:
bitfields, the lazy-records generator, benchmarks and copy removal
(`a-parsed-record-is-copied-on-its-way-out.md`), and the cleanup including the
naming sweep. Freestanding comes after.

The developer wrote "c++ standard freestanding which is much easier so v1". That
may mean freestanding's own first scope rather than the January release, which
needs confirming. Everything below assumes post-v1.

## Considerations the discussion surfaced

**What is settled.**

- **Target:** the standard freestanding subset.
- **Fixed-capacity containers:** "same behavior as today, we do not what yet
  another schema for the fixed sized containers". `max_bytes` stays the only
  bound, and the schema gains no new surface.
- **`optional` and `variant`** are in scope.
- **The std-stream adapters** stay inside the one single header, behind
  preprocessor switches.
- **`AGENTS.md`'s target statement** gains a freestanding axis when freestanding
  lands, not before.
- **Generator, long term:** a hand-written generator for the C++20 tier, and
  `std::generator` for C++23 and up, chosen by tier. v1 starts with
  `std::generator`.
- **MSVC freestanding detection is on hold** (below).
- **`value()` without exceptions** is flagged by the developer for discussion.
- **The preprocessor rule now in `AGENTS.md`:** the preprocessor records
  build-wide choices once: the language tier and "whether the environment is
  hosted". Code paths are chosen with `if constexpr`, `if consteval` and
  concepts.

**What the standard requires is C++26; what libraries provide is earlier.**

- **The standard.** The C++26 tables mark `std::array`, `string_view`,
  `optional` and `variant` partially freestanding (P2407R5), and `expected` and
  `span` as well (P2833R2). The adoption votes weren't found and should be
  checked. Below C++26, C++23 (P1642) adds `<utility>`, `<tuple>`, and parts of
  `<ranges>`/`<iterator>`/`<functional>`/`<memory>`. So the developer's belief
  that optional and variant are freestanding holds in the standard from C++26
  on.
- **libstdc++ 14.** On this machine, under `-ffreestanding`, it accepts
  `<optional>`, `<variant>` and `<expected>` at both `-std=c++20` and
  `-std=c++23`, and rejects `<vector>` and `<iostream>`. At C++20, `<expected>`
  includes but provides nothing, because its contents are C++23-guarded.
- **Elsewhere.** On libc++ and MSVC's STL, availability below C++26 is unknown.

**The real list is cheap to test.** The amalgam includes 22 standard headers
(`single_header/s2s.hpp:1-22`). Five have been tried under `-ffreestanding`. The
other 17 are untested:

- `<array>`, `<string_view>`, `<span>`, `<algorithm>`, `<cstring>`,
  `<functional>`, `<ranges>`, `<limits>`, `<bit>`, `<utility>`
- `<concepts>`, `<type_traits>`, `<cstdint>`, `<cstddef>`
- `<string>`, `<cstdio>`, `<cassert>`

v1's cleanup removes `<cstdio>`, `<cassert>` and `<cstring>`, which are unused,
and `<string>` from `typelist.hpp`. One loop over the full list, per library and
per tier, would replace this estimate with a fact.

**Throwing members are where "partially freestanding" bites.** P2407's partial
classes leave out members that throw. Whether that includes `std::get` on a
variant (which throws `bad_variant_access`) and `optional::value()` should be
checked.

s2s calls `std::get<idx>` at `derived_value.hpp:128` and `field_writer.hpp:339`.
The tests and docs use `std::get` on variance fields throughout, at around 45
sites in `test/`.

s2s's own expected, "identical to c++23" in topic 1, has the same problem with
`value()`, which throws `bad_expected_access`. The developer has flagged that as
a discussion item. Whatever it settles for the expected probably settles
`std::get` and `optional::value()` too.

**Detecting hosted: `__STDC_HOSTED__` is reliable on gcc and clang.** Tested on
this machine:

- g++-13, g++-14 and clang++-19 define it as 1 by default, and 0 with
  `-ffreestanding`.
- A 2009 clang bug, where it was always 1, is fixed.
- The pitfall: it is defined as 0, never left undefined, so the test must be
  `__STDC_HOSTED__ == 0`, never `#ifndef`.

**MSVC detection is on hold.** Known: MSVC is not verified, and I know of no
equivalent to `-ffreestanding` there. Unknown: what MSVC defines
`__STDC_HOSTED__` as, whether any MSVC configuration counts as freestanding, and
so whether a user macro is the only signal on MSVC. The developer has put this on
hold, so it is recorded here rather than decided.

Under the `AGENTS.md` rule, "hosted" is recorded once from these signals, and code
paths select on it with `if constexpr` and concepts. Whether a user macro can
override `__STDC_HOSTED__`, and which wins if they disagree, is open.

**`if constexpr` cannot hide what a freestanding build lacks.** The std-stream
adapters `std_read_trait`/`std_write_trait` name `std::istream&` and
`std::ostream&` (`stream_traits.hpp:19,24`). A template's non-dependent names
are looked up at definition, so a discarded `if constexpr` branch that names
them is still ill-formed when `<iostream>` is absent. The adapters, and
`#include <iostream>`, need a namespace-scope `#if` on the hosted choice. The
rule permits that: it is a build-wide choice, and only `#if` inside functions is
forbidden.

`scripts/amalgam.py` hoists every `#include <...>` to the top of the single
header unconditionally (lines 110-111, 120). Keeping the adapters "behind
preprocessor switches inside the single header" therefore requires `amalgam.py`
to preserve conditional includes. Topic 1 needs the same change.

**What ties s2s to the hosted library today.**

- **iostreams.** `stream_traits.hpp:6` includes `<iostream>`. `read_trait` takes
  a `std::streamsize` (line 28), which is an `<ios>` type. The header is reached
  from `read_impl.hpp` through `address_manip.hpp`, and from every `api/` header,
  the view included (`view/struct_view.hpp:18`).
- **Allocation.**
  - `std::vector`/`std::string` are hard-wired in several places:
    - `vec_field`, `vector_of_records` and `str_field`
      (`api/field_descriptors.hpp:67-90`);
    - the `as_vec`/`as_str` tags (`type_tags.hpp:83-113`);
    - the recognising traits (`type_traits.hpp:100,157,176,217`) and
      `field_traits.hpp:129,149`;
    - `byte_addressof` (`address_manip.hpp:28-60`).
  - Containers grow at `read_impl.hpp:81,124` and `field_reader.hpp:182,219`.
  - The code itself asks how a user could supply their own vector
    (`field_descriptors.hpp:66`), and `docs/reference.md:265-267` lists this as a
    known limitation.
- **Public types.**
  - every read entry point returns `std::expected<T, cast_error>`;
  - `cast_error::failed_at` is a `std::string_view`;
  - `maybe` holds `std::optional<T>` (`field.hpp:73`);
  - `variance` holds `std::variant` (`helper.hpp:36`).

**Without containers, a freestanding schema loses three descriptors.**
`<vector>` and `<string>` are not freestanding at any tier. Unless the container
concept lands with freestanding, a freestanding build has no `vec_field`,
`str_field` or `vector_of_records`. In practice, freestanding needs the concept.

**The container concept already has an implicit contract.** The engine uses:

- `value_type`
- `resize(n)`, `clear()` and `push_back(x)`
- `size()`
- `data()`, or `&obj[0]` for a string (`address_manip.hpp:34`)
- `operator[]` and range-for

The constexpr path fills elements one at a time (`read_impl.hpp:82-90`).
"Concepts, not SFINAE" requires naming it, and selecting behaviour by concept is
what the preprocessor rule prefers. `std::vector`, `std::pmr::vector`,
`etl::vector` and a fixed-capacity s2s vector would all satisfy it.

The copy brief's candidates touch these same operations. Reading each record
element straight into `emplace_back()` instead of a temporary, for example,
would add `emplace_back` and `back` to the contract.

**Two front ends, one concept.** The DSL names the container through a
descriptor parameter. The C++26 annotated front end names it as the member type,
for example `etl::vector<u8, 64> payload;`. Both reach the backend as the `T` in
`field<id, T, ...>`, so a concept on that `T` serves both.

**A fixed capacity keeps today's behaviour, which leaves one gap.** A wire
length within `max_bytes` but above capacity reaches `resize` on the caller's
type:

- `etl::vector` asserts or truncates, depending on configuration;
- s2s's own `static_vector` has no capacity check in `push_back` (lines 17-20).

Whether s2s may check `capacity()` when the concept offers one is still open. It
would be engine behaviour, not schema surface.

**`include/lib/`'s `static_vector` is not ready to be public.** It has:

- no `resize`, `clear`, `data` or `value_type`;
- an unchecked `push_back`;
- an ill-formed `N = 0`.

A public container is user-facing API, which is a higher bar than the
`include/lib/` override's "readable errors" justification.

**Expected in a freestanding build.** Topic 1 settles on `std::expected` from
C++23 up and s2s's own below. On libstdc++ 14, `<expected>` survives
`-ffreestanding` at C++23, so the rule holds. On a library where it doesn't, a
freestanding C++23 build would need s2s's own expected, which contradicts "its 20
or 23 from s2s perspective".

**Streams for firmware already exist in shape.** `constexpr_stream`-derived
streams stage through `std::array<char, N>` and need no iostream. Two already
exist: `test/utils/constexpr_memstream.hpp` and the view's
`unchecked_byte_source` (`view/struct_view.hpp:42`). What is missing is a
`read_trait` that doesn't name `std::streamsize`.

**The generator plan puts the freestanding-capable one in the C++20 tier.** In v1
the generator is `std::generator`. The developer wants it heap-free "to fit
embedded world". `std::generator`'s allocator parameter can keep the frame off
the global heap. But whether `<generator>` is freestanding at all is unverified,
and probably not.

The long-term hand-written generator for the C++20 tier needs only
`<coroutine>`, which is freestanding even in C++20. Its promise can own frame
allocation. That makes it the natural freestanding generator, even at C++23, and
that sits awkwardly with "chosen by tier". Freestanding may be a second axis
that picks the generator.

Even a heap-free frame yields records that hold whatever containers the schema
names. Until the container concept exists, "without heap" holds only for
schemas without container fields.

**Allocation-free enforcement and the copy benchmarks share machinery.** The copy
brief counts allocations, for example with a replaced global `operator new`, as
the deterministic stand-in for copies. A freestanding guarantee that "this schema
does not allocate" is the same count with zero as the expected value. How it is
enforced is still unanswered, but v1's benchmark work may build the tool.

**Bitfields cost nothing here.** Shift-and-mask over an integer needs nothing
hosted.

**Freestanding meets what `AGENTS.md` says the project is.** "Target is hosted
C++23" and "Domains: None … the embedded domain rules do not apply to it". The
freestanding axis gets added when freestanding lands. Whether "Domains: None"
changes then is open.

**What this means for January.** Freestanding is not in v1. v1 touches it in
three places:

- the cleanup removes unused hosted includes;
- the benchmark work may produce the allocation counter;
- the v1 `std::generator`'s frame allocation is decided while it is being
  written.

The 22-header test takes about an hour and could be run at any time.

## Directions rejected

**Practical embedded (a full toolchain with no heap and no exceptions) as the
target.** The developer chose the standard's freestanding subset. Being
heap-free is still wanted for the generator, as a property in its own right.

**A capacity-driven schema option for fixed-size containers.** "We do not what
yet another schema for the fixed sized containers".

**A full s2s standard library for freestanding below C++26.** On libstdc++,
optional, variant and expected already work under `-ffreestanding`.

**An allocator template parameter on the standard containers.** It does not
cover `etl::vector`, and `<vector>` is not freestanding anyway.

**Replacing `std::vector` with an s2s container in the descriptors.** It breaks
every hosted user.

## Questions still open

- Does "so v1" for freestanding mean the January release, or the first scope of
  freestanding work after it?
- How is allocation-free enforced? Which test fails when a schema allocates?
  (Unanswered; the copy brief's allocation counter may be the tool.)
- Can a user macro override `__STDC_HOSTED__`, and which wins if they disagree?
  MSVC detection itself is on hold.
- Do the 17 untested headers survive `-ffreestanding` on libstdc++? What about
  libc++ and MSVC, at each tier?
- Are `std::get` on variant and `optional::value()` available freestanding,
  given that they throw? This rides on the flagged `value()` discussion.
- With no new schema surface, may s2s check `capacity()` before `resize`?
- Does the container concept land together with freestanding?
- In a freestanding C++23 build, which generator is used: `std::generator`, if
  `<generator>` is available, or the hand-written one? Is freestanding a second
  axis that picks it?
- Can `read_trait` drop `std::streamsize` without breaking existing custom
  streams?
- When the freestanding axis is added to `AGENTS.md`, does "Domains: None"
  change?
