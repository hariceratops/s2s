# A real compile-time test tier, on qlibs/ut

## Overview

The test tree splits `test/runtime/` (GoogleTest) from `test/constexpr/`
(bare `static_assert`s) — a split by *how* a test executes, not by *what* it
covers. That split hid a real hole: the read-side compile-time tier
compile-time-tests two `u32` fields and one buffer-exhaustion case, against
46 `static_assert`s on the write side, even though `docs/compile-time.md`
claims compile-time parsing for the whole schema language. This work adopts
[qlibs/ut](https://github.com/qlibs/ut) for everything pure (its
`"name"_test = [] constexpr { … }` bodies run at both compile time and run
time), reorganises the test tree around features instead of execution mode
so each feature's coverage reads in one place, and fills the read-side
compile-time gap the reorganisation exposes. GoogleTest keeps everything the
compiler cannot evaluate on its own — the file-based read tests and the
`stringstream`-based write tests alike.

## Goals

- `ut` is fetched via `FetchContent` (`URL` at a tagged release, `URL_HASH
  SHA256`, `DOWNLOAD_NO_EXTRACT TRUE` — the header ships as a file named
  `ut` with no extension) and wired into the build as an interface target.
  The untracked root copy `qlib_ut.hpp` stays deleted.
- The test tree is reorganised by feature: `test/fields/<feature>_read.*` and
  `test/fields/<feature>_write.*`, one group per schema construct (trivials,
  fixed/variable strings, fixed/variable arrays, arrays/vectors of records,
  optional/maybe, variance/union, constraints, computed/derived fields,
  containers, field-list metadata). The unit that makes a feature's coverage
  read in one place is the **directory**, not a single file: within a
  feature and direction, GoogleTest and `ut` live in separate sources
  compiled to separate binaries (`optional_read.cpp` beside
  `optional_read_ct.cpp`). The split that disappears is by execution mode,
  not by direction — read and write stay separate.
- **GoogleTest and `ut` never share a binary.** Two concrete conflicts, both
  verified against the `ut` header:
  - `ut`'s reporter signals failure from its destructor via
    `__builtin_abort()`. Combined with GoogleTest, `RUN_ALL_TESTS()` returns
    and prints a passing summary, `main` returns cleanly, and static
    destruction then aborts the process — CTest reports a subprocess abort
    after the output said everything passed.
  - `ut`'s compile-time mode is `static_assert((run.test(), …))` inside the
    translation unit, so a failing compile-time test is a compile error that
    takes the whole TU with it — including that feature's GoogleTest cases,
    which then cannot run at all.
  Separate sources and separate targets avoid both. `ut` ships
  `UT_COMPILE_TIME_ONLY` and `UT_RUN_TIME_ONLY`, so one `ut` source can build
  as two targets and give CTest a distinct entry per mode.
- The framework boundary is mechanical, not conventional: **constant-
  evaluability** routes a test. A body the compiler can evaluate on its own
  goes to `ut` and runs in both modes; anything else stays with GoogleTest.
  The compiler enforces this rather than a reviewer — a body that cannot be
  constant-evaluated fails to compile in a `ut` compile-time target, so a
  mis-routed test is a build error, not a slow drift. No dual copies — a case
  that moves to `ut` is deleted from GoogleTest, not duplicated.

  An earlier draft of this spec used "touches a file" as the rule. That was
  wrong, and the correction is recorded because the mistake is easy to repeat:
  only the seven read-side runtime files use `ifstream`. All eight write-side
  runtime files use `std::stringstream`, which touches no file but is equally
  not constant-evaluable, so the file-based rule routed the entire write-side
  runtime suite to `ut`, where it cannot run at all — and that suite is the
  only coverage of the runtime `writeable` overloads `AGENTS.md` requires.
- Consequence of the corrected rule, recorded so the slices are not
  mis-sized: of ~118 existing GoogleTest cases, only `field_validators.cpp`'s
  seven actually become `ut` cases. This work is therefore overwhelmingly
  *new* compile-time coverage plus a file reorganisation, not a migration of
  existing cases between frameworks.
- The read-side compile-time tier reaches the same feature breadth as the
  write side's 46 `static_assert`s — roughly one `ut` suite per row of the
  `docs/schema/` field-kind table. Done means: **no schema construct listed
  in `docs/schema/` lacks at least one compile-time case.** Individual test
  cases are enumerated during implementation, not fixed here.
- `test/constexpr/static_tests_1.cpp` is rewritten against the current API
  as an `ut`-based suite, registered in the build, absorbing the `todos.md`
  item "Static test cases for all type traits and concepts." This forces a
  decision — made during implementation, not here — on the orphaned
  `include/field_validation/field_value_constraints_traits.hpp`: wire it
  into a header that reaches the amalgamated single header, or delete it.
- `test/compile_fail/` and `test/single_header/` are renamed to names that
  state what they verify (negative compilation; doc-example/single-header
  binding) rather than their mechanism. Exact names are a design decision.
  The `test/single_header/` rename is a docs-wide sweep, not a `git mv`: it
  touches every `<!-- docs: test/single_header/… -->` binding across
  `docs/` (~14 files), `AGENTS.md` line 36, and the path baked into
  `test/single_header/CMakeLists.txt`'s `doc_examples_match` /
  `docs_nav_lists_every_page` test registrations. `ctest` enforces the
  binding, so a missed reference fails the build rather than rotting
  silently — the sweep is complete when `ctest` is green.
- `test/hello/` moves to `examples/` at the repo root. It gains a CMake
  target and a CTest entry — it is currently registered nowhere, which is
  how it went unbuilt and unnoticed; the move must not repeat that.
- GoogleTest cases moved into a feature file get a per-feature suite name as
  a side effect of the move (the catch-all `S2STest` suite stops meaning
  anything once tests live under `test/fields/<feature>_read.cpp`), using
  the write side's existing behavior-sentence case names
  (`ByteswapsPerElementNotPerBuffer`) as the style to converge on. This is
  scoped to names the move makes nonsensical — not a systematic naming pass
  over the whole suite.
- **The migration must not change what is covered, only how it is
  organised.** Any pass/fail difference introduced by a file move or a
  `static_assert` → `ut` port is a bug in that port, fixed before the move
  counts as done. Newly-added read-side compile-time coverage is the one
  sanctioned change to the result set, and it only adds.

## Non-Goals

- Migrating GoogleTest's stream-driven tests to `ut` — neither the
  `ifstream`-based read tests nor the `stringstream`-based write tests. They
  are not constant-evaluable, so there is nowhere to migrate them to; a
  framework chosen for its constexpr support cannot run them.
- A framework-only split (GoogleTest for runtime, `ut` for compile time).
  Rejected in the originating session: it quietly recreates the
  split-by-execution-mode problem this work exists to remove.
- Vendoring `ut`. GoogleTest already comes in via `FetchContent`, so builds
  already require network at configure time; vendoring buys nothing and a
  vendored copy drifts from whatever is pinned.
- A systematic pass over read-side test naming beyond what the feature-file
  move forces. Real, but a separate piece of work.
- Expanding `test/compile_fail/` beyond its current one case. Real coverage
  hole (a library whose value is compile-time diagnostics has exactly one
  negative test), but a separate piece of work — only its name changes here.
- Fixing the exact replacement names for `test/compile_fail/` and
  `test/single_header/` at this level. Design's call.
- Enumerating individual read-side compile-time test cases here. The PRD
  fixes the required breadth (parity with the write side, by schema
  construct); implementation enumerates the cases.
- A go/no-go gate on whether `ut`'s compile-time diagnostics actually help
  on this codebase's deep template chains. The decision to adopt `ut` is
  settled; see Open Questions for how this is tracked as a risk instead.

## Technical Approach

- C++23, header-only library, CMake + CTest, GoogleTest and (new) `ut` both
  via `FetchContent`. No new runtime dependency — `ut` is test-only.
- `ut`'s dual-mode property (`"name"_test = [] constexpr { … }` runs under
  both constant evaluation and at run time) is what lets a single suite
  file cover a feature's pure behaviour once instead of twice (a
  `static_assert` block plus a parallel GoogleTest block).
- Directory shape: `test/fields/` holding, per schema construct and
  direction, a GoogleTest source and a `ut` source compiled to separate
  binaries (`optional_read.cpp` + `optional_read_ct.cpp`, and likewise for
  `_write`); renamed `test/compile_fail/` and `test/single_header/`
  equivalents; `examples/` at repo root (outside `test/`); `test/utils/`
  (existing helpers: `constexpr_memstream.hpp`, `s2s_test_utils.hpp`) likely
  stays as shared support for both frameworks. Per-directory `CMakeLists.txt`
  follows the existing pattern of a helper function per test kind — alongside
  the current `add_constexpr_test`, an `add_ut_test` registering a `ut`
  source, plausibly twice (once under `UT_COMPILE_TIME_ONLY`, once under
  `UT_RUN_TIME_ONLY`) so each mode is its own CTest entry and a compile-time
  break is distinguishable from a runtime one.
- Suggested sequencing, since this lands as multiple vertical slices rather
  than one change (dependencies noted so slicing has something to work
  against):
  1. Wire up `ut` via `FetchContent`; prove the interface target and one
     trivial dual-mode suite build and run under CTest. No reorg yet.
  2. Reorganise the existing tree into `test/fields/`: move current
     `runtime/*.cpp` and `constexpr/*.cpp` content into the new per-feature
     files, port existing pure `static_assert`s to `ut` suites 1:1 (no new
     coverage), rename GoogleTest suites/cases as forced by the move. Gate:
     the non-negotiable above — identical pass/fail behaviour before and
     after, verified per feature before moving to the next.
  3. Fill the read-side compile-time gap, per schema construct, until no
     row of `docs/schema/`'s field-kind table is uncovered. This is
     additive on top of step 2's reorganised files and is the bulk of the
     new test-writing effort.
  4. Rewrite `static_tests_1.cpp` as a registered `ut` suite against the
     current API; resolve the orphaned
     `field_value_constraints_traits.hpp` (wire in or delete).
  5. Rename `test/compile_fail/` and `test/single_header/`; sweep every
     `docs/`, `AGENTS.md`, and CMakeLists reference to the old
     `test/single_header/` path; confirm `ctest` (in particular
     `doc_examples_match` and `docs_nav_lists_every_page`) is green.
  6. Move `test/hello/` to `examples/`, add its CMake target and CTest
     entry.
  Steps 1–2 block everything else. 3–6 are independent of each other and
  can land in any order once 2 is done.
- Constraint carried from `AGENTS.md`: new stream-touching code needs both
  a `constexpr_stream` overload and a runtime `readable`/`writeable`
  overload, and coverage in both `test/runtime/`-equivalent and
  `test/constexpr/`-equivalent form — this PRD's reorg changes *where* that
  dual coverage lives (one feature directory, not two execution-mode
  directories), not the requirement itself.

## Open Questions

- **Does `ut`'s compile-time failure output actually help on this
  codebase's deep template chains, or does it degrade to the same template
  soup as `static_assert`?** Not gated — the decision to adopt `ut` is
  settled — but worth an early throwaway check, and it carries more weight
  than first assumed. `ut`'s compile-time mode is `static_assert`
  underneath, so it does **not** avoid halting the build at the first
  compile-time failure; that advantage holds for its runtime mode only. The
  reasons `ut` beats bare `static_assert` are therefore two, not three:
  named tests with a real message (making "what do we cover at compile
  time" answerable), and the same body runnable at runtime to see actual
  values when the compile-time diagnostic is opaque. Both concern
  diagnostics and reporting, so a bad result here weakens the rationale more
  than a three-reason framing suggested. C++26's user-generated
  `static_assert` messages (P2741) would narrow the first of the two.
- **Exact replacement names** for `test/compile_fail/` and
  `test/single_header/` — left to design. (The CMake mechanism for combining
  the two frameworks is no longer open: they do not share a binary. See the
  Goals bullet on binary separation.)
- **`field_value_constraints_traits.hpp`**: wire it into a header the
  amalgamation reaches, or delete it — left to implementation, once
  `static_tests_1.cpp` is rewritten and it's clear which traits still have
  meaning against the current API.
