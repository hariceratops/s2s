# [chore] README shrinks to a pitch; Brief Tour deleted

"Brief Tour" duplicates `UserGuide.md`'s struct and descriptor sections almost
verbatim, and the two have already drifted. With the guide now owning all
reference material, the README becomes a pitch and stops being a second,
competing copy of the reference.

Depends on: 013, 023.

## Acceptance Criteria
- The "Brief Tour" section is deleted, not maintained — along with the "for a
  brief tour, refer the section" pointer under "API documentation" that
  introduces it.
- The README's shape is: what s2s is, the two compile-time claims in the order
  fixed by 013, the feature list, requirements and compiler support, one read
  example, one write example, the roadmap, and a link to the guide.
- No struct or descriptor tables and no `struct_cast`/`struct_write` signatures
  remain in `README.md`; those live only in `UserGuide.md`.
- Grepping both files for the descriptor alias list and for the
  `struct_cast`/`struct_write` signatures finds each exactly once across the two
  files.
- Every example remaining in the README is backed by a registered CTest target
  under `test/single_header/`.
- `ctest` passes.
