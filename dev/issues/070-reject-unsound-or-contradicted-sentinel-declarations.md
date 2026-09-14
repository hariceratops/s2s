# [feat] reject a sentinel declaration that cannot synthesise or is contradicted

069 builds the read surface for `until_field_equals`. This slice adds the two
compile-time rejections a schema author can hit, held as their own slice on
the companion's 064 precedent: the read surface being correct does not mean
every declaration using it is sound, and a wrong declaration should fail to
compile rather than emit a corrupt file the first time someone writes it.

**The declaration is contradictory.** `vector_of_records` accepting both
`until_field_equals<...>` and a size-axis count (a literal, a sibling field,
or a callable) states two different things about what ends the run. There is
no sensible way to honour both, so the combination is a hard compile error.

**The declaration cannot synthesise its own terminator.** Write has to emit a
terminator after the last real element by default-constructing an element,
setting its named field to the sentinel, and writing it. That only produces
the right bytes when every other field of the element has a *determined*
width in that state: each is either fixed-width, or has its length driven —
directly or transitively — by the named field. A terminating element whose
other fields have independent widths would emit a full-size element instead
of a one-byte (or otherwise minimal) sentinel, silently producing a wire
format the schema does not describe. This is the corruption class the
feature's write-side design exists to avoid, and it belongs at compile time:
a schema that cannot synthesise correctly must not compile, not fail
quietly on the first write.

Deliberately not the cheaper positional rule ("the named field must be the
element's first field"), which rejects a legal schema whose sentinel field
follows a fixed-width one. Deliberately not left unchecked either, which
reintroduces exactly the silent-corruption class discarding the terminator
was meant to prevent.

Spec: `dev/specs/a-record-sequence-cannot-end-at-a-sentinel.md`.

## Acceptance Criteria
- A `vector_of_records` declaring both `until_field_equals<...>` and a
  size-axis count fails to compile, with a `test/must_not_compile/` entry.
- A compile-time synthesis-soundness concept verifies that every field of the
  terminating element other than the named one is fixed-width or has its
  length driven, directly or transitively, by the named field.
- A schema violating the soundness check fails to compile as a hard error,
  with its own `test/must_not_compile/` entry.
- Both witness schemas from 069 (GIF sub-block, DNS label) pass the soundness
  check as a positive control — asserted in `test/schema/`, not
  `must_not_compile`.
- **Each `must_not_compile` case is checked against a no-`CASE` control build
  and confirmed to fail on its own diagnostic before registration** — an
  entry that fails for an unrelated reason passes just as green, the trap 047
  fell into and the companion's 064 guarded against the same way.
- No case pins compiler diagnostic text; the concept names are the
  diagnostics, per the project's existing policy
  (`must_not_compile/byte_order_misuse.cpp`'s header).
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
