# [chore] README: lead with the right compile-time claim, add a write example

The README's single "constexpr! as much as possible" bullet reads as a
precondition, so a reader whose schema has a `std::string` stops reading. And
the README has no write-direction example at all, which was the complaint that
started this effort. Neither depends on the UserGuide restructure, so both land
early rather than behind ~1200 lines of prose.

Depends on: 012.

## Acceptance Criteria
- The opening states "parser generated at compile time" as an unconditional
  claim — true for every user, including with a plain `std::ifstream` — before
  "parsing performed at compile time", which is stated as requiring a
  `constexpr_stream` and no allocating fields.
- This is a rewrite of the opening paragraph and the feature list's constexpr
  bullet, not a new subheading bolted on beside them.
- The README carries a write-direction example as a full compiling code block
  with the same prominence as the existing read example — not a one-line
  mention.
- The write example is adapted from `test/single_header/roundtrip.cpp` rather
  than written from scratch, and is backed by a registered CTest target.
- `ctest` passes.
- "Brief Tour" is left untouched by this issue; it is deleted in 024, once
  `UserGuide.md` owns the reference material it duplicates.
