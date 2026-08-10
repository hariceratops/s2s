# [chore] Reference section and ToC reconciliation

Closes `UserGuide.md`. The table of contents currently promises sections that do
not exist — twelve read-side field kinds, "Using custom stream", "Validating
field members". By this point those sections exist, so the contents page is
reconciled against reality rather than deleted.

Depends on: 014, 015, 016, 017, 018, 019, 020, 021, 022.

## Acceptance Criteria
- A Reference section carries the corrected, fenced descriptor alias list.
- `UserGuide.md`'s table of contents contains no heading without a matching
  section, and no section exists without a table of contents entry.
- The contents reflect the spine's order: Schema → Constraints and validation →
  Reading → Writing → Streams → Compile time → Reference.
- The "[Work in Progress]" marker in the H1 is resolved — removed, or replaced
  with a statement of what is actually still outstanding.
- The two `// todo` comments inside the descriptor block (getting vector length
  in bytes, user-supplied vector implementation or allocator) are either
  restated as known limitations in prose or dropped — not left as stray
  implementation notes in user-facing reference material.
