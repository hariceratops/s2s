# [test] Rewrite static_tests_1.cpp as a registered ut traits suite

`test/constexpr/static_tests_1.cpp` is unregistered in CMake and written
against an API several generations old (`field<"x", int, 4>` for today's
`field<id, T, field_size<...>, constraint>`, `typelist::typelist` for
`typelist::list`). It asserts over traits in
`include/field_validation/field_value_constraints_traits.hpp`, which no header
includes, so those traits never reach the amalgamated single header at all.

This absorbs the `dev/inbox/todos.md` item "Static test cases for all type
traits and concepts" and forces the decision on the orphaned header. It also
takes in the two constexpr files that test machinery rather than a schema
construct: `containers.cpp` and `field_list_metadata.cpp`.

Depends on: 028.

## Acceptance Criteria
- The traits and concepts suite lives under `test/fields/` (or a sibling for
  non-descriptor machinery, if design prefers) as a ut source, registered via
  `add_ut_test`, and runs in both modes.
- Every assertion is rewritten against the current API. No assertion is carried
  over unmodified without confirming it still names something that exists.
- `containers.cpp` and `field_list_metadata.cpp` are folded in as ut suites,
  and their existing `static_assert`s port 1:1.
- `static_tests_2.cpp`'s `meta::type_id` assertions are folded in or explicitly
  retained, not silently dropped.
- `field_value_constraints_traits.hpp` is resolved: either included from a
  header the amalgamation reaches — in which case its traits are covered by
  this suite — or deleted. The choice and its reasoning are recorded in a
  `## Notes` section.
- The `todos.md` traits item is ticked off, with a pointer to this issue.
- No unregistered test source remains under the old `test/constexpr/`.
- `ctest` green.

## Notes 2026-08-12

Done. `ctest` 104/104 (was 98; +10 entries, −4). **`test/constexpr/` is gone**,
and with `test/runtime/` retired in 037, no execution-mode directory remains.
The tree is feature-first end to end.

Three ut suites under `test/internals/`, which is where the design puts
machinery that is not a schema construct:

| New | Replaces | Tests / asserts |
|---|---|---|
| `traits_ct.cpp` | `static_tests_1.cpp` + `static_tests_2.cpp` | 10 / 44 |
| `field_list_metadata_ct.cpp` | `field_list_metadata.cpp` | 7 / 47 |
| `containers_ct.cpp` | `containers.cpp` | 7 / 26 |

`field_list_metadata.cpp`'s 47 `static_assert`s port to exactly 47 ut asserts.
`static_tests_2.cpp`'s five `meta::type_id` assertions are folded into
`traits_ct.cpp` unchanged, not dropped.

### `static_tests_1.cpp` could never have compiled

It was not merely stale. Four of its assertions are two contradictory pairs:

```cpp
static_assert(is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_with_runtime_size_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
static_assert(!is_field_v<field<"hello", int, runtime_size<from_field<"a">>>>);
```

X and !X on the same type. The whole `tl::any_of_v`/`tl::all_are_same_v` block
also appears twice, verbatim. Being unregistered is what let that sit.

Assertions dropped because the names no longer exist, each checked against
`include/` rather than assumed: `are_unique_types_v`, `has_unique_field_ids_v`
(now the concept `has_unique_field_ids` over field types, plus
`are_field_ids_unique` over a `std::array<std::string_view, N>` — the latter is
asserted), `is_field_with_runtime_size_v`, `runtime_size`, `from_field`,
`is_field_v`, `maybe_field`. `typelist::typelist` is now `typelist::list`.

`basic_field` requires an integral `T`, so the old `field<"x", float, 4>`
assertions use the `field` template directly.

### Two orphaned headers, both deleted

`include/field_validation/field_value_constraints_traits.hpp` is what this
issue asked about. It defines `is_no_constraint_v`, no header includes it, so
it never reaches the amalgam, and nothing in `include/` consults it. It is not
in `docs/reference.md` either. **Deleted** rather than wired in: adding an
include so a trait no caller uses could be asserted would be manufacturing the
coverage.

`include/lib/metaprog/typelist_manip.hpp` was found the same way and is worse.
Also included by nothing, also absent from the amalgam — and it **does not
compile**. Every specialisation names `typelist::typelist<...>`, which has not
existed since the template was renamed to `list`:

```
error: 'typelist' is not a member of 's2s::typelist'; did you mean 's2s::typelist'?
```

Probed directly rather than inferred. That is where `variant_from_tlist_v`
lived, so `static_tests_1.cpp`'s assertion on it is dropped with the header.
Deleting a header that cannot be included cannot break a caller. Outside this
issue's stated scope — flagged rather than filed, since filing an issue to
delete a file nothing can include is ceremony.

### `containers.cpp` asserted nothing

All nine of its `static_assert`s were commented out. Its `main` printed a
`static_map` and two `static_vector`s to `cout` and returned 0 — a registered
CTest entry that could not fail, the fourth false green in this feature.
`containers_ct.cpp` asserts the behaviour it was printing: construction, size
and capacity, `push_back` order, `sort_ranges` over integers and over string
views, and `static_map` lookup including a missing key. All 7 run at compile
time.

`todos.md`'s "Static test cases for all type traits and concepts" is ticked
with a pointer here. "Remove dead code" stays open — this removed two headers,
not the item.
