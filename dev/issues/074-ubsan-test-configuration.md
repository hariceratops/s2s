# [chore] a UBSan-instrumented test configuration with one local command

x86 does not trap on a misaligned typed load, so a view that regressed to
reading through a `const T*` would still produce correct values there. The only
thing that catches it is UBSan's alignment check, run over the view's run-time
tests. The repo has no sanitizer flags anywhere: no `CMakePresets.json`,
`CMakeLists.txt` sets only `-Wall -Wextra -Wpedantic` and the per-build-type
flags, and the only CI workflow builds the docs. This is therefore new
infrastructure, and it has to exist before the slices that need it.

This slice adds the configuration and proves it on the existing suite; it adds
no view code. Views' own tests join it as they land (075 onward, and 081 for
the alignment cases that exist to be run under it).

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- One documented command, runnable locally, configures, builds and runs the
  run-time tests under `-fsanitize=undefined` (which includes alignment). The
  mechanism — default in the developer build, a CMake option, or a preset — is
  the implementer's choice, but the single command is binding and is written
  down where a developer will find it (`AGENTS.md` and/or `docs/`).
- Sanitizer reports are fatal (`-fno-sanitize-recover=all` or equivalent), so a
  report makes the CTest entry fail rather than print and pass.
- It runs the GoogleTest suite and the `ut` run-time entries. The
  `*_compile_time` entries are excluded: they only drive `cmake --build` and
  gain nothing from instrumentation.
- Applies to the gcc and clang toolchains. MSVC has no equivalent and is
  exempt; the configuration does not break an MSVC configure.
- The existing suite passes under it, or any pre-existing UBSan report it
  surfaces is listed in the issue file with its location rather than silently
  suppressed.
- Clang configures and builds the tree, except `delimited_write_ct` (see
  Findings: clang 19 with libstdc++ 14 cannot constant-evaluate the
  `std::string` that test builds, with or without the sanitizer; tracked
  separately). Today it cannot configure at all: the doc-example,
  shipped-header and example targets pass `-Werror -Wno-non-template-friend`,
  a GNU-only option clang rejects as an error. The flag becomes GNU-only here,
  since the clang half of this configuration is otherwise unreachable
  (design `dev/design/reading-two-fields-costs-a-whole-record.md`, F4).
- The default (unsanitized) build and `ctest` are unchanged and green.

## Findings

Verification log:

- Default build (g++-14, no sanitizer) and `ctest`: 209/209 passed; the
  `run_time` label selects 66 of them.
- First g++-14 workflow attempt with the build preset at `"jobs": 0` failed:
  `0` means an unbounded `make -j`, and cc1plus processes were OOM-killed
  ("Killed signal terminated program cc1plus") on this 4-core/15 GB host.
  Not a UBSan report. The preset now uses `"jobs": 4`.
- g++-14 `CXX=g++-14 cmake --workflow --preset ubsan --fresh`: configure,
  build and test succeed; 66/66 `run_time` tests passed, zero "runtime error"
  lines, no `*_compile_time` entry run. Flags confirmed in flags.make:
  `-O2 -g -fsanitize=undefined -fno-sanitize-recover=all`. No pre-existing
  UBSan report on the existing suite.
- Fatality proof (g++-14): a temporary gtest doing a `const uint32_t*` load at
  `buf + 1` was added to `test/schema/trivial_write.cpp`. The ubsan run
  reported `runtime error: reference binding to misaligned address ... requires
  4 byte alignment`, `trivial_write` Failed, 65/66 passed, ctest rc 8. The
  probe was then removed (file restored byte for byte; `git diff` of
  `test/schema/` is empty).
- clang++-19: configure succeeds (the GNU-only `-Wno-non-template-friend` fix
  works: the doc-example, shipped-header and example targets build). With
  `make -k` the whole tree builds except `delimited_write_ct` (all three
  variants: compile_time, run_time, coverage). Cause, pre-existing and
  independent of UBSan (reproduced in a plain unsanitized clang++-19 build):
  `test/schema/delimited_write_ct.cpp:193` builds
  `std::string("fo\x00o", 4)` in a `constexpr` ut test, and clang 19 with
  libstdc++ 14 cannot constant-evaluate `basic_string::_M_construct`
  ("undefined function ... cannot be used in a constant expression"). So the
  documented `CXX=clang++-19 cmake --workflow --preset ubsan --fresh` stops at
  the build step today; it is not a UBSan report and was not suppressed.
  Of the other clang targets, `ctest -L run_time` reported 66/66 passed, but
  that count includes `delimited_write_ct_run_time` running a stale g++-14
  binary, so only 65 are genuinely clang-built. Not fixed here: the spec's
  design (F4) only anticipated the `-Wno-non-template-friend` blocker.
