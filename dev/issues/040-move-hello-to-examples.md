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

## Notes 2026-08-12

Done. `ctest` 106/106 (was 104; +2 entries — the example and its fixture).

`test/hello/` is gone. `examples/` exists at the repo root with
`hello_s2s.cpp`, `make_sample.py` and its own CMakeLists, wired into the root
`CMakeLists.txt` alongside `test/`.

### What was wrong with it

Three separate things, and the second is why the first two survived:

1. It read `sample.bin`, which nothing in the repo generates. On every run the
   `ifstream` failed to open.
2. It ended in an unconditional `return 0` after a `transform_error` that only
   printed. So the failing path above reported success.
3. It was registered in no CMakeLists, so nothing built it, ran it, or
   compiled it against the current API.

Rewritten to return 1 when the file will not open and 1 when the cast fails,
printing the reason and the offending field to stderr. `sample.bin` is written
by `make_sample.py`, registered as a `FIXTURES_SETUP` entry the way
`test/as_shipped/make_doc_fixtures.py` already handles the README's read
example.

### The entry was confirmed to test something

Running the built binary in a directory without the fixture prints
`cannot open sample.bin` and exits 1. Since CTest regenerates the fixture from
`make_sample.py` before each run, the thing to perturb is the generator, not
the file it writes — truncating the payload there gave:

```
106 - hello_s2s (Failed)
read failed: reason=0 at=text
```

Reverted afterwards. Perturbing the built `sample.bin` instead would have
proved nothing: the setup entry overwrites it.

The example builds with `-Wall -Wextra -Wpedantic -Werror` against
`single_header/s2s.hpp` only, matching the other single-header consumers, and
compiles clean against the current API without changes beyond the error
handling. `AGENTS.md` now describes `examples/` and the rule that an example
must be able to fail.
