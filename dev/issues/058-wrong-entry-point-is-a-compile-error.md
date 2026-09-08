# [feat] the wrong entry point for a schema shape is a compile error

`struct_cast` and `struct_cast_le`/`struct_cast_be` describe two different kinds
of format, and a call that names the wrong one says something untrue about the
file. Both directions are rejected at compile time; there is deliberately no way
to force a fixed order over a self-announcing schema.

Constraints go on named concepts, per project convention, so the failure reads
as an unsatisfied constraint naming the schema shape rather than as a wall of
substitution failures.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- `struct_cast<T>` on a schema with no announcing record fails to compile, with
  a diagnostic naming the mismatch and pointing at `struct_cast_le`/
  `struct_cast_be`.
- `struct_cast_le<T>`/`struct_cast_be<T>` on a schema that declares an
  announcing record fails to compile, with a diagnostic naming the mismatch and
  pointing at `struct_cast`.
- Both constraints are expressed as named concepts.
- `must_not_compile` cases cover both directions and match against expected
  output.
- Every existing call site of `struct_cast_le`/`struct_cast_be` still compiles
  unmodified.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

Depends on 054.
