# [chore] Regenerate the amalgamated single header

`single_header/s2s.hpp` is what consumers actually include, so the write path
is not shipped until it is folded in. The new headers must satisfy what
`scripts/amalgam.py` expects when flattening `include/`.

Depends on: 001–009.

## Acceptance Criteria
- Every new write-path header uses `#ifndef _NAME_HPP_` / `#define _NAME_HPP_`
  include guards (not `#pragma once`) and includes its siblings by relative
  path, matching what the amalgamator expects.
- `scripts/amalgam.py` regenerates `single_header/s2s.hpp` with no errors,
  warnings, or duplicated declarations.
- The regenerated single header compiles standalone against C++23 with
  `-Wall -Wextra -Wpedantic` and no new diagnostics.
- A program including only `single_header/s2s.hpp` can call
  `struct_write_le`/`struct_write_be` and round-trip a struct.
- The regenerated header is committed, matching the repo's existing practice
  of shipping the generated artifact.
- No cyclic inclusion is introduced between the write-side headers and the
  existing read-side headers.
- **`scripts/amalgam.py` produces deterministic, machine-independent output.**
  It currently does neither, which makes the tracked
  `single_header/s2s.hpp` unreviewable: regenerating it on an unchanged tree
  produces a 237-line diff with no semantic change. Two causes:
  - Absolute paths from the generating machine are baked into the `// Begin` /
    `// End` provenance comments (the committed header carries
    `/home/hari/repos/s2s/...`; regenerating elsewhere rewrites every one).
    Emit repo-relative paths instead.
  - The block of `#include <...>` std headers is emitted in nondeterministic
    order, so it reshuffles between runs. Sort it.

  Fix both before the write path lands, or issue 010's own diff will be
  impossible to review.
