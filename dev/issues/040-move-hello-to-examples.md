# [chore] Move test/hello/ to examples/ and build it

`test/hello/hello_s2s.cpp` is not a test — it is an example program, and it is
registered in no CMakeLists at all, so nothing builds or runs it. That is how
it went stale unnoticed. It moves to `examples/` at the repo root, outside
`test/`, and gains a target so the same thing cannot happen again.

Depends on: 028. Independent of 029-039.

## Acceptance Criteria
- `examples/` exists at the repo root, outside `test/`, and holds the example.
- `test/hello/` is gone.
- The example has a CMake target and a CTest entry — building is not enough;
  it must run, since an example that compiles but crashes is still broken.
- **The example must be capable of failing.** As written it ends in an
  unconditional `return 0` after a `transform_error` that only prints, so it
  reports success on every path — including the path it currently takes, since
  it opens a `sample.bin` that nothing in the repo generates. A CTest entry
  against it would pass while the example did nothing. Fix both: return
  non-zero on the error path, and make the input exist (generate the fixture
  at build time, as `test/single_header/make_doc_fixtures.py` already does for
  the doc examples, or embed the bytes).
- Deliberately corrupting the input fixture once makes the CTest entry fail,
  confirming the entry tests something. Then reverted.
- The example builds against `single_header/s2s.hpp` and depends on
  `generate_single_header`, matching how the other single-header consumers are
  wired.
- The example compiles and runs clean against the current API. If it does not,
  it is updated — this move must not carry a broken example forward.
- `AGENTS.md` mentions `examples/` where it describes the repo layout.
- `ctest` green.
