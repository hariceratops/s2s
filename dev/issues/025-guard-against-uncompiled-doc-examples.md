# [test] Guard against uncompiled documentation examples

The PRD requires that no example ships as prose only, but nothing enforces it.
Per-issue acceptance criteria cover the thirteen slices while they are open;
they do nothing about the example someone pastes into `UserGuide.md` a year
from now. The broken include path this restructure exists partly to fix is
exactly that failure, already realised once — it survived because no test ever
compiled the block it sat in.

`test/single_header/check_deterministic.cmake` is the precedent: a property the
repo cares about, asserted by CTest rather than trusted to reviewers.

Despite its number this runs early — directly after 012, before the examples of
013 onward exist — so that the convention is in place while they are written
rather than retrofitted across thirteen slices. Ordering here comes from the
`Depends on` lines, not from the file numbering.

Depends on: 012.

## Acceptance Criteria
- A CTest test fails when a non-trivial fenced example in `README.md` or
  `UserGuide.md` has no corresponding compiled source under
  `test/single_header/`.
- It also fails in the other direction that matters: when a doc example and the
  source backing it have drifted apart in content, not merely when the file is
  missing. A guard that only checks existence would have passed the whole time
  the README's include path was wrong.
- The binding between a doc block and its source is explicit in the files
  themselves rather than inferred by fuzzy matching — a delimited region in the
  `.cpp` paired with a marker on the doc block, or an equivalent mechanism that
  a reader can follow by eye.
- What counts as "non-trivial" is defined and applied consistently: signature
  and declaration snippets illustrating an API shape are exempt, complete
  programs are not.
- The check runs in the same CTest invocation as the rest of the suite; no
  separate manual step.
- The failure message names the offending example and what to do about it,
  rather than reporting a diff of the whole file.
- Issues 013 through 024 conform to the convention this establishes.
