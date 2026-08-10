# [chore] Compile time — the two claims, explained

Issue 013 states the two compile-time claims in the corrected order in the
README. The guide is where they get explained, including the conditions the
narrower one carries — which is what the current single "constexpr! as much as
possible" bullet leaves a reader to guess at.

Depends on: 013, 021.

## Acceptance Criteria
- "Parser generated at compile time" is explained as unconditional: what the
  template machinery produces, and that there is no runtime dispatch over the
  schema. This holds with a plain `std::ifstream`.
- "Parsing performed at compile time" is explained as the narrower claim: it
  requires a `constexpr_stream` and a schema with no allocating fields, and both
  conditions are stated plainly rather than implied.
- Which field kinds disqualify a schema from constexpr parsing is stated
  concretely: `vec_field`, `str_field`, `vector_of_records` — anything that
  allocates.
- The toolchain constraint is stated where a reader hits it: runtime use works
  on gcc 13.1, but constexpr use of `struct_field_list` needs gcc 14 or newer,
  and `CMakeLists.txt` fails configure below it.
- A constexpr example is backed by a registered CTest target under
  `test/single_header/`, with `static_assert`s that prove the parse happened
  during constant evaluation.
