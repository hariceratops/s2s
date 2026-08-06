- [x] Size dependency resolution checks
- [x] Parse dependency resolution checks
- [ ] Static test cases for all type traits and concepts
- [ ] Remove dead code
- [ ] Refactor type and non-type list
- [x] Value based TMP
- [x] Support all major compilers = msvc, indirectly supports more compilers = gcc 12.1
- [ ] Fix compiler errors in all major compilers
- [ ] Simplify union field implementation
- [ ] Remove outdated todos
- [ ] Refactor type deduction with fold expressions
- [ ] Rename stream_wrapper_impl
- [ ] Refactor and remove cyclic inclusions
- [ ] Split cast_impl into cast_api and cast_impl

## Error ergonomics (backlog, deferred 2026-08-07)

`cast_error` carries one field name, which is why two acceptance criteria are
recorded as unmet on issues 006 and 007:

- a violation inside a nested record names the **outermost** record field, not
  the inner one (`records`, not `records.inner.tag`);
- a shared-length contradiction names the length field but not the dependent
  that disagreed with it.

Both need the same change and neither is worth doing piecemeal. Context
gathered before deferring, so picking this up again is cheap:

- Only 4 sites construct a `cast_error`; ~41 read `failed_at`, nearly all in
  tests plus the README and UserGuide samples.
- The two cases want structurally different information. Nesting is a chain
  (`records → inner → tag`); a length contradiction is two peers at the same
  level. One list serving both means the caller disambiguates via
  `failure_reason`; separate fields are self-describing but fatter.
- Whatever holds the names must be fixed-capacity — nothing allocates and it
  has to work under constant evaluation — so it is a
  `static_vector<std::string_view, N>` with N chosen at compile time, and
  every `cast_error` pays that size on both directions.
- The inner name is discarded at exactly two symmetric places:
  `field_writer.hpp:102` and `field_reader.hpp:178`. Fixing only the write
  side would leave the directions asymmetric.
- Open sub-question: whether `failed_at` survives as the primary name or is
  replaced by `path[0]`.

Design §5 rejected a second field name as out of scope, and §5.1 records the
one enumerator that decision has since been reversed for
(`found_contradicting_length`). Revisiting this supersedes both.

## Smaller items left open by the write feature

- [ ] `-Wnon-template-friend` fires from the friend-injection trick for
      `type_id` (`include/lib/metaprog/mp.hpp:16`) on **every** consumer build
      with warnings enabled. Predates the write path; suppressed narrowly in
      `test/single_header` so that guard stays sensitive to new diagnostics.
- [ ] `scripts/amalgam.py:25,131` catch bare `Exception`. Predates the write
      path; `OSError` is what both sites actually handle.
- [ ] "No heap allocation introduced" on issues 001, 002 and 004 was asserted
      from reading the code, never measured. No allocator hook or static check
      exists anywhere in the repo.
- [ ] `is_computed_size_v` lives in `field_size.hpp`, a header both directions
      include, but exists purely for the write path.
- [ ] `include/field_write/derived_value.hpp` now holds four obligation kinds
      plus both `derive_value` and `verify_conditional_len`. Half the file
      verifies rather than derives; `obligations.hpp` would name it better.
