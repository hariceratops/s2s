# [fix] Fix the two documentation defects

Two concrete defects sit alongside the documentation gap and are independent of
the restructure. Fixing them first means the rendered docs are correct while the
larger restructure is still in progress.

## Acceptance Criteria
- `UserGuide.md`'s descriptor alias block (currently lines 65-106) is wrapped in
  a `cpp` fence and renders as a code block rather than as reflowed prose.
- `README.md`'s constexpr example includes `test/utils/constexpr_memstream.hpp`,
  the path that exists on disk, instead of `test/constexpr_memstream.hpp`.
- No other content changes in either file.
