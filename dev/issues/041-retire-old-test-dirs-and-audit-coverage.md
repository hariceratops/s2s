# [test] Retire test/runtime/ and test/constexpr/, audit the coverage bar

Closing slice. The feature slices each move their own tests; this one confirms
nothing was left behind and verifies the spec's hard done-bar — **no schema
construct listed in `docs/schema/` lacks at least one compile-time case.**

This is the only place the bar is checked as a whole. Each feature slice
asserts it for its own descriptors; a construct owned by no slice would
otherwise slip through unnoticed.

Depends on: 029-038. Independent of 039 and 040.

## Acceptance Criteria
- `test/runtime/` and `test/constexpr/` no longer exist, and `test/CMakeLists.txt`
  no longer references them.
- Every row of the descriptor table in `docs/schema/index.md` is walked
  against the ut suites, and each row is shown to have at least one
  compile-time case. The mapping — row to suite — is recorded in a `## Notes`
  section so the next person can re-check it without rederiving it.
- Every row of the form table in `docs/schema/size-axis.md` is walked the same
  way.
- Any row found uncovered is either covered here or recorded as a deliberate
  exception with a reason; "we ran out of time" is not a reason.
- No test source anywhere is unregistered in CMake.
- No GoogleTest binary links `ut`, and no `ut` binary links GoogleTest.
- The claims in `docs/compile-time.md` are re-read against what the suites
  actually cover, and corrected if the page overstates or understates.
- `ctest` green on a clean configure.
