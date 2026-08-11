# [chore] Reading, brought to parity with Writing

The inversion this whole effort exists to correct: the older, complete,
most-used direction has the thinner documentation. `UserGuide.md` currently has
no Reading section at all — only a table of contents promising twelve read-side
field kinds that were never written.

Depends on: 014, 015, 016, 017, 018.

## Acceptance Criteria
- A "Reading" section exists with a compiling worked example, matching the shape
  "Writing to stream" already has.
- Every field kind the current table of contents promises is covered: trivials,
  array of trivials, array of records, length-prefixed vector of trivials,
  length-prefixed vector of records, const-sized strings, length-prefixed
  strings, optionals, unions, magic strings, magic numbers.
- Read semantics that are not shared with the write direction are stated: what a
  partially consumed stream means, and what the caller gets back on failure.
- A full error table gives, for each way a read can fail, the `failure_reason`
  and what `failed_at` names — including that a failure inside a nested record
  names the outermost record field, matching the write path.
- No descriptor, size, or constraint reference material is restated here; the
  section points at Schema for anything direction-neutral.
- The worked example is backed by a registered CTest target under
  `test/single_header/`.
