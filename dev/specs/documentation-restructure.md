# PRD: Documentation Restructure (README + UserGuide)

Flavor: **standard** (s2s is a public/OSS header-only library with external
consumers, a README, and a `UserGuide.md`; not a personal-scale tool).

## Overview
s2s's documentation is structurally inverted: the read direction (older,
complete, most-used) is nearly unwritten, while the write direction (newer)
is fully documented — because the docs have been organized by direction
rather than by the schema language that actually drives both. A working
session held immediately before this one (`dev/inbox/documentation-gaps.md`,
2026-08-09) inventoried the gap against the code, not against impression:
roughly 13+ public constructs (type deduction for unions, presence
deduction, computed sizes, an eleven-construct constraint DSL, the stream
concepts, three descriptors) have no prose anywhere in `README.md` or
`UserGuide.md`; `UserGuide.md`'s table of contents promises sections
(twelve read-side field kinds, "Using custom stream", "Validating field
members") that do not exist in the file; and two concrete defects — an
unfenced code block and a broken README include path — sit alongside the
gap. This PRD scopes a restructure of s2s's two documentation surfaces
around the schema-centric spine that session proposed (field descriptors as
a direction-neutral schema language, not a read-side feature), fixes the
two defects, and keeps every non-trivial example compiler-verified the way
`test/single_header/doc_example.cpp` already keeps the README's example
honest. All five open questions raised by the source session have since
been settled by the user; see Decisions below.

## Goals
- Every public construct exported through `include/s2s.hpp` gets prose
  somewhere in `README.md` or `UserGuide.md`: `type_switch`, `match_case`,
  `if_else_ladder`, `parse_if`, `predicate`, `len_from_fields`,
  `size_from_fields`, `size_choices`, `compute` (including its generic form
  `compute<callable, R, Fs>` — see Decisions §1), the eleven-construct
  constraint DSL (`eq`/`neq`/`lt`/`gt`/`lte`/`gte`/`any_of`/`range`/
  `is_in_open_range`/`is_in_closed_range`/`no_constraint`), the stream
  concepts (`read_trait`/`write_trait`/`constexpr_read_trait`/
  `constexpr_write_trait`/`input_stream_like`/`output_stream_like`/
  `constexpr_stream`), the descriptors `struct_field`/`c_arr_field`/
  `c_str_field`, and `with_fields` (the combinator underneath `parse_if`,
  `len_from_fields`, and `compute` itself — see Decisions §1).
- `fixed_string` and `as_fixed_string` from `include/lib/` are documented as
  supported surface (see Decisions §2); the rest of `include/lib/`
  (`static_vector`, `static_map`, the metaprogramming helpers) is stated
  explicitly as internal, not covered.
- Documentation organized around the schema language as the shared,
  direction-neutral spine — four axes (`id`/`type`/`size`/`constraint`) plus
  two combinators (`maybe`, `variance`) — not duplicated per direction.
  `UserGuide.md`'s table of contents either matches sections that actually
  exist by the end of this work, or is removed; no promised-and-missing
  headings survive.
- The read direction reaches parity of depth with the write direction: a
  compiling worked example, per-field-kind/per-size-kind treatment, a full
  error table, and contract/semantics prose — matching what "Writing to
  stream" already has.
- The README's opening leads with "parser generated at compile time"
  (unconditional — everyone gets it, including with a plain `std::ifstream`)
  and states "parsing performed at compile time" (needs `constexpr_stream`,
  no allocating fields) as the second, narrower claim. This is a rewrite of
  the opening paragraph, not a subheading — the current single "constexpr!
  as much as possible" bullet reads as a precondition and causes readers
  with a `std::string` field to stop reading.
- Both concrete defects are fixed: the missing opening code fence around
  the descriptor block (currently `UserGuide.md` lines 65-107), and
  `README.md`'s broken include path
  (`test/constexpr_memstream.hpp` -> `test/utils/constexpr_memstream.hpp`).
- The README shrinks to a pitch: what s2s is, the two compile-time claims,
  the feature list, requirements/compiler support, one read example, one
  write example (see Decisions §4), the roadmap, and a link to the guide.
  The "Brief Tour" section is deleted, not maintained. `UserGuide.md` owns
  all reference material exclusively — no content is duplicated across the
  two files.
- Every non-trivial example introduced or moved by this restructure is
  backed by a file under `test/single_header/` that compiles and runs as a
  registered CTest test, following the existing
  `test/single_header/doc_example.cpp` pattern. No example ships as prose
  only. The README's write example reuses or adapts the existing
  `test/single_header/roundtrip.cpp` rather than introducing a new example
  from scratch.
- Streams get a real, in-doc reference implementation of what
  `read_trait`/`write_trait` (and the constexpr pair) require, rather than
  the current pointer into the test tree.

## Non-Goals
- No new library features, API changes, or behavior changes. This is a
  documentation-only restructure; any defect found in code (not docs)
  during this work is out of scope and gets filed separately.
- No documentation site generator (mkdocs-material or otherwise) as part of
  this PRD. The Schema section is written as plain markdown in
  `UserGuide.md` first; the generator question is revisited only once its
  real size is known (see Decisions §5). No nav-tree scaffolding, no CI
  Pages workflow, no requirements file for a generator — all deferred.
- `static_vector`, `static_map`, and the `include/lib/` metaprogramming
  helpers are explicitly excluded from documentation (see Decisions §2) —
  stated as internal, not silently omitted.
- No versioning/changelog process for documentation changes.
- No translation/localization of docs.

## User Stories
- As a new s2s user evaluating the library, I want the README's opening
  claim to lead with what I get "for free" (compile-time-generated parsing,
  no runtime dispatch) rather than reading the constexpr-parsing bullet as
  a precondition, so I don't stop reading because my schema has a
  `std::string` in it.
- As an s2s user reading from a stream (the common case), I want prose for
  every field kind and size kind with a worked example and full error
  table, matching what the write direction already has, so I'm not forced
  to read `include/` headers to use the older, complete half of the
  library.
- As an s2s user writing a custom stream type, I want a documented list of
  what `read_trait`/`write_trait` (or their constexpr counterparts)
  require, backed by a real reference implementation, so "pluggable
  interfaces working with custom streams" is something I can act on rather
  than a headline.
- As a first-time README reader, I want a write-direction example with the
  same prominence as the read examples, so I don't conclude the library
  only reads and miss that serialization exists — this was the complaint
  that started this effort.
- As a maintainer, I want every example in the restructured docs backed by
  a compiled test under `test/single_header/`, so documentation drift (like
  the current broken include path) gets caught by CI instead of by a future
  reader.

## Technical Approach
- **Scope**: `README.md` and `UserGuide.md` only, plus the test files the
  new/moved examples require under `test/single_header/`. No new files
  beyond what the spine's sections need, and no site-generator scaffolding.
- **Sequencing**: write the Schema section (and the rest of the spine) as
  plain markdown in `UserGuide.md` first; revisit mkdocs-material only once
  the real prose size is known post-restructure. The source session's
  estimate — roughly 1200 lines across ~7 sections, up from 526 today — is
  a figure to validate against the actual result, not a target to hit.
- **Structural spine**: Schema (four axes + descriptor table + two
  combinators + size axis in full + type deduction + presence deduction,
  including `with_fields`/`compute` and its `parse_if`/`len_from_fields`
  aliases) → Constraints and validation → Reading → Writing → Streams →
  Compile time → Reference (the corrected, fenced alias list). This spine
  lives entirely in `UserGuide.md`; the README does not carry a parallel
  version of it (Decisions §3).
- **README shape**: pitch, both compile-time claims in the order fixed
  above, feature list, requirements/compiler support, one read example, one
  write example (adapted from `roundtrip.cpp`), roadmap, link to the guide.
  No "Brief Tour" section, no descriptor table, no struct/cast signatures —
  those live only in `UserGuide.md`.
- **Examples stay compiled**: every worked example gets a matching file
  under `test/single_header/`, registered as a CTest target the way
  `doc_example.cpp` and `roundtrip.cpp` already are.
- **Defect fixes are mechanical**: add the fence around `UserGuide.md`'s
  descriptor alias block (lines 65-107); correct `README.md`'s stream
  header include path (line 81).
- No code, API, build, or test-infrastructure changes beyond the
  new/adjusted `test/single_header/*.cpp` example files and their CMake
  registration.

## Success Metrics
- Every construct named in `dev/inbox/documentation-gaps.md` §2 (13+ items),
  plus `with_fields` and the generic `compute` form, appears with
  explanatory prose (not just a signature or a passing mention) somewhere
  in `README.md` or `UserGuide.md`.
- `fixed_string` and `as_fixed_string` are documented; a documentation-scope
  statement exists naming `static_vector`, `static_map`, and the metaprog
  helpers as internal/uncovered.
- `UserGuide.md`'s table of contents contains no heading without a matching
  section, and no section exists without a ToC entry.
- The read direction's documentation reaches the same shape as the write
  direction's: a worked example, an error table, and contract/semantics
  prose all present.
- `ctest` passes with every prose example in the restructured docs backed
  by a corresponding target under `test/single_header/` — no example
  exists only as markdown.
- The two named defects are gone: the descriptor block in `UserGuide.md`
  renders as a fenced code block; `README.md`'s include path matches a file
  that actually exists on disk (`test/utils/constexpr_memstream.hpp`).
- The README's opening states "parser generated at compile time" before
  "parsing performed at compile time", with the first framed as
  unconditional and the second as requiring `constexpr_stream`.
- The README contains a write-direction example with the same prominence
  (a full compiling code block, not a one-line mention) as its read
  example.
- No reference content — struct/descriptor tables, cast signatures, field
  kind lists — appears in both `README.md` and `UserGuide.md`. Grepping
  both files for the descriptor alias list, the `struct_cast`/`stream_cast`
  signatures, etc. finds each exactly once.

## Decisions
The five questions the source session (`dev/inbox/documentation-gaps.md` §4)
left open have been settled by the user. Recorded here so the rationale
travels with the PRD rather than living only in chat history.

1. **`compute`/`predicate` are public API, documented in full — not an
   `api/` implementation leak.** Evidence from the test tree: `with_fields`
   appears 46 times, `parse_if` 18, `predicate` 14, `compute` directly 5.
   `parse_if` and `len_from_fields` cannot be written without naming
   `with_fields`, so the family is already load-bearing public surface. The
   widest option was chosen: the generic form `compute<callable, R, Fs>`
   gets equal billing as a user-facing mechanism for deriving any value
   from sibling fields, not just the `bool`/size aliases. **This is a
   stability commitment** — documenting the generic form obligates the
   project to keep its signature stable going forward, not just the
   `parse_if`/`len_from_fields` aliases built on top of it.
2. **`include/lib/` supported surface is `fixed_string` and
   `as_fixed_string`; everything else is explicitly internal.**
   `fixed_string` is unavoidable public surface: `magic_string` and
   `fixed_string_field` use it as the field type, so assigning to a magic
   field means constructing one (the UserGuide's own write example already
   does: `obj["magic"_f] = s2s::fixed_string<3>("S2S")`). It appears 11
   times in user-facing test code. `static_vector`, `static_map`, and the
   metaprogramming helpers have zero user-facing uses and are stated
   explicitly as internal and not covered — a stated non-goal, not silence.
3. **README shrinks to a pitch plus two examples; `UserGuide.md` owns all
   reference material exclusively.** The README becomes: what s2s is, the
   two compile-time claims in corrected order, the feature list,
   requirements/compiler support, one read example, one write example, the
   roadmap, and a link to the guide. The "Brief Tour" section is deleted,
   not maintained — it currently duplicates the UserGuide's struct and
   descriptor sections almost verbatim and the two have already drifted.
   No reference content appears in both files (see Success Metrics).
4. **The write direction gets an equal-prominence README example.** This
   is what triggered the whole effort: the opening complaint was that the
   README has no example showing the write direction exists.
   `test/single_header/roundtrip.cpp` already exists and already compiles
   as a registered CTest target; the README example reuses or adapts it
   rather than being written from scratch.
5. **Site generator: deferred, as a decision, not left open.** The Schema
   section is written as plain markdown in `UserGuide.md` first; the
   generator question is revisited only once its real size is known.
   Picking the container before the contents exist produces a nav tree of
   empty stubs. If a generator is adopted later it will be mkdocs-material
   (mdBook rejected — no Rust in this project, so it buys nothing but
   familiarity for the wrong audience). `scripts/amalgam.py` already makes
   `python3` a build dependency, so the eventual cost of adopting
   mkdocs-material is a requirements file plus a Pages workflow, not a new
   toolchain. Site tooling itself stays in Non-Goals for this PRD.
