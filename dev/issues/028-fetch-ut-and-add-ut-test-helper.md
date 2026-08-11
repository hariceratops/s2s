# [chore] Fetch qlibs/ut and add the add_ut_test helper

Bring qlibs/ut into the build as a test-only dependency and establish the
CMake pattern every later slice uses. No test is reorganised here — this slice
exists so the following ones have somewhere to put a dual-mode suite.

ut and GoogleTest never share a binary: ut's reporter aborts from its
destructor, and its compile-time mode is `static_assert` inside the TU, so a
compile-time failure would take a combined binary's GoogleTest cases down with
it. The helper enforces that separation.

Blocks: 029-041.

## Acceptance Criteria
- `ut` is fetched via `FetchContent_Declare` with `URL` at a tagged release,
  `URL_HASH SHA256`, and `DOWNLOAD_NO_EXTRACT TRUE` — the header ships as a
  file named `ut` with no extension. No `GIT_REPOSITORY` at a branch.
- The pinned tag and hash are recorded in the CMake, not left to resolve at
  configure time.
- An interface target carries the include path so a source can `#include <ut>`.
- `add_ut_test(<name>)` registers one ut source as two targets and two CTest
  entries: one built with `UT_COMPILE_TIME_ONLY`, one with `UT_RUN_TIME_ONLY`,
  so a compile-time break is distinguishable from a runtime one.
- **The compile-time entry drives a build, not a binary.** Registering it as
  `add_test(COMMAND <binary>)` is a false green: when the source stops
  compiling, the build fails, the stale binary from the last good build
  survives, and `ctest` runs it and reports a pass. The entry must invoke
  `cmake --build --target <t>` — the idiom `compile_fail/` already uses — with
  `RESOURCE_LOCK` so `ctest -j` cannot race two entries into the same build
  tree. Verified in the design; do not simplify this back.
- Shared declarations (GoogleTest's `FetchContent`, `add_struct_cast_test`)
  are hoisted out of `test/runtime/CMakeLists.txt` into `test/CMakeLists.txt`
  here. Every later slice strips those directories, so they must not own
  anything the rest of the tree needs.
- The compile-time-only build still needs a `main` — ut tests are statements
  in a function, and the macro removes the runtime block and the reporter
  destructor, not the requirement for an entry point.
- No `add_ut_test` target links GoogleTest, and no GoogleTest target includes
  `ut`.
- A pilot dual-mode suite (`"name"_test = [] constexpr { … }`) builds and
  passes under both CTest entries.
- Deliberately failing the pilot's expectation once confirms the compile-time
  entry fails at compile time and the runtime entry reports a named failure —
  then reverted.
- The spec's open question about template soup is already answered: the design
  captured a real diagnostic (test name, exact failing line, expected value,
  plus one line of `struct_field_list` soup — usable, not clean). No need to
  re-derive it; confirm the pilot matches and move on.
- The pin is already chosen: tag `v2.1.6`, `URL_HASH SHA256=80afe19304e2401d
  f66f842a3698a87a56d694572014a0e32a51dc854e58ac01`. Re-verify the hash against
  what is downloaded rather than trusting this line.
- `qlib_ut.hpp` is absent from the repo root.

## Notes 2026-08-12

Done. `ctest` 46/46 on a clean configure (gcc 14.2, CMake 3.28.3).

The pilot is `test/internals/harness_ct.cpp` — a smoke test of the tier
itself (ut + `memstream` + s2s at compile time), not feature coverage. It
stays; `test/internals/` is where the design puts machinery that is not a
schema construct.

Verification actually run, not assumed:

- Breaking one expectation fails the compile-time target with ut's named
  diagnostic, and fails the run-time target with
  `harness_ct.cpp:41:FAILED:"a little endian trivial pair round trips":
  3405697037 == 3405697038`, exit 134. The run-time mode printing the actual
  value is the advantage that justified adopting ut; confirmed on this
  codebase, not just in the design's sample.
- **The false green is real and the fix works.** With the source broken and a
  stale `harness_ct_compile_time` binary from the previous good build still on
  disk, `ctest -R harness_ct_compile_time` fails. Had the entry been
  `add_test(COMMAND <binary>)` it would have run the stale binary and passed.
- No `qlibs_ut` target links gtest and no gtest target sees ut's include path;
  checked in the generated `link.txt` and `flags.make`, not by reading CMake.

Hash verification is not a separate step — `URL_HASH` makes CMake fail the
configure on a mismatch, and the configure succeeded.

Also landed here, both from the design and both cheaper once than in nine
slices: `add_struct_cast_test` now gives each GoogleTest binary its own
working directory (18 of them wrote the same `test_input.bin`, so `ctest -j`
was unsafe), and `constexpr_memstream.hpp` includes
`include/stream/stream_traits.hpp` instead of the single header, so ut sources
no longer drag the generated amalgam into their diagnostics.
