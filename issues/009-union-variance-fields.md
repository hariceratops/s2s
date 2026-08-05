# [feat] Union (variance) fields round-trip

Support `variance<id, type_deducer>` for both type-deduction mechanisms, which
differ fundamentally in invertibility. `type_switch` maps a discriminant value
to an alternative through `match_case`s, so the discriminant is derivable from
the held alternative. `if_else_ladder` selects via arbitrary `branch`
predicates, which cannot be inverted and can only be verified.

Depends on: 001, 005 (extends the read-only mechanism), 006.

## Acceptance Criteria
- A `type_switch`-based `variance` field round-trips, with the discriminant
  **derived** from the variant's held alternative via the corresponding
  `match_case` value — the stored discriminant is not trusted.
- **Add a `has_unique_match_values` compile-time check** alongside the existing
  `has_unique_field_choices` on `variance`. The existing check does *not* give
  value uniqueness: `extract_field_choices` collects `type_tags` and discards
  `match_values` (`include/type_deduction/type/type_deduction_metafunctions.hpp:38`),
  so it enforces unique case *types*. Unique types is what makes
  alternative-to-value inversion well-defined, but duplicate *values* silently
  break round-trip fidelity: writing alternative `B` emits value `v`, and
  reading `v` back selects the first matching case `A`. A schema with duplicate
  match values must fail to compile.
- The discriminant keeps its own schema field slot — its bytes must be emitted
  or the stream will not read back, and `match_field` resolves against that
  slot on read. Only the *source* of the written value changes.
- The discriminant field of a `type_switch`-based `variance` is treated as a
  derived field by 005's compile-time predicate: assigning to it through the
  non-const `operator[]` does not compile, while the const read still does.
- An `if_else_ladder`-based `variance` field round-trips when the stored
  discriminant data and the held alternative agree.
- When the ladder's branch predicates, evaluated against the struct being
  written, select a different alternative than the one actually held, the
  write fails with a validation-style `cast_error` naming the union field.
- Fields feeding `if_else_ladder` branch predicates remain assignable.
- Union alternatives that are themselves record types recurse correctly per 006.
- Resolves the spec's open question: document in the design whether a
  `type_switch` discriminant still requires its own schema field slot at write
  time, or is inferred purely from the held alternative — and state how that
  interacts with `field_reader.hpp`'s existing `deduce_type` /
  `type_deduction_guide` on the read side, so round-trip stays symmetric.
- Round-trip and mismatch cases verified in `test/runtime/`; round-trip also in
  `test/constexpr/`; the discriminant compile-fail case joins 005's
  compile-fail test.
