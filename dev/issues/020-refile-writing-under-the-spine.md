# [chore] Refile Writing under the spine

"Writing to stream" is the strongest material in the repo — a compiling worked
example, the rationale for why derivation exists, a check-to-`failure_reason`
table, and an explicit write-once/fail-fast contract. Its problem is filing, not
quality. Once Schema owns the direction-neutral material, the write section
keeps only what is genuinely write-specific.

Depends on: 014, 015, 016, 017, 018.

## Acceptance Criteria
- The Writing section sits in its place in the spine after Reading, with its
  worked example, "Derived fields are read-only", "What is checked at write
  time", and "Write-once, fail-fast" content preserved.
- Material now owned by Schema — what `len_from_field`, `type_switch`,
  `parse_if`, and `if_else_ladder` *are* — is removed from the write section and
  referenced instead. What stays is what those constructs *do on write*.
- The derived-versus-verified distinction stays stated in full here, since it is
  write-specific: which fields the library derives and ignores the struct's
  value for, and which it verifies against what the struct holds.
- The breaking-change note for read-side code that assigned to a length field
  survives the move.
- `test/single_header/doc_example.cpp` still compiles and passes; if the worked
  example moved or changed, that file moved or changed with it.
- No content in this section is duplicated in Schema.
