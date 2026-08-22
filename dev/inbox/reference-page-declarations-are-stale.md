# `docs/reference.md`'s declaration block is stale

Found on 2026-08-19 while landing
`dev/issues/052-variance-carries-a-constraint.md`, which had to correct one
line of that block and left the rest.

Not a PRD. A defect note — the work is mechanical once someone decides how
the page should be kept honest, which is the only open question here.

## What is wrong

The page opens with:

> The descriptor aliases as declared in `include/api/field_descriptors.hpp`.
> The [descriptor table](schema/index.md#the-descriptors) is the readable
> form; this is the exact one.

It has not been the exact one since
`dev/issues/045-new-surface-and-atomic-migration.md` made a descriptor's
trailing parameters an order-independent pack of `auto` NTTPs. Every entry
in the block still shows the pre-045 positional form:

```cpp
template <fixed_string id, integral T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
  requires field_fits_to_underlying_type<size_type, T>
using basic_field = field<id, T, size_type, constraint_on_value>;
```

against what the header actually declares:

```cpp
template <fixed_string id, integral T, field_option_like<T> auto... opts>
  requires field_fits_to_underlying_type<size_of_pack<T, opts...>, T>
using basic_field = field<id, T, size_of_pack<T, opts...>, constraint_of_pack<T, opts...>>;
```

The page is worse than merely out of date, because it is half-migrated and
so reads as though it had been maintained. Three entries — `fixed_array_field`,
`c_arr_field`, `c_str_field` — already say `constraint_of_pack<...>` on their
right-hand side while keeping a positional `auto constraint_on_value` on the
left, which is not a signature that has ever existed.

Divergences beyond the pack itself, all in the same block:

- `str_field`, `vec_field` and `vector_of_records` lost their `max_bytes`
  parameter — the page predates `dev/issues/047-bound-container-allocation-with-max-bytes.md`
  entirely, so nothing there suggests a field has an allocation ceiling.
- `struct_field` is shown as taking no constraint. It takes one.
- `magic_number` is shown constraining `fixed_size_like size`. The source
  constrains `fixed_size_like<size_type_of<size>>` in a requires-clause,
  because a class-type NTTP cannot be constrained in place.
- `fixed_string_field` and `magic_string` are shown yielding `fixed<N + 1>`.
  There is no `fixed` spelling; it is `byte_count{...}`.

`variance` is correct as of 052 and is the only line that is.

## Why it went unnoticed

Nothing checks it. `ctest` enforces that every *complete program* in the docs
compiles and matches its source (`doc_examples_match`) and that every page is
in the nav (`docs_nav_lists_every_page`), but this block is a bare fence with
no `<!-- docs: -->` binding, so no target owns it. It is the one place in the
documentation that claims to reproduce source text verbatim and the one place
with no mechanism to make that true.

That is the real finding. The stale text is a symptom, and re-transcribing it
by hand reintroduces the same failure the moment the next slice changes a
descriptor — 045, 047 and 052 each changed one and none of the three noticed
the page.

## The decision to make

Three options, in the order they seem worth considering:

1. **Generate the block from the header**, the way `doc_examples_match`
   already binds a fenced block to a source region. The declarations sit
   contiguously in `field_descriptors.hpp`; a marker pair around them and a
   check comparing the fence to the extracted text would make drift a test
   failure. Most work, and the only option that stays true on its own.
2. **Re-transcribe it and drop the "exact" claim**, describing the shapes
   instead of reproducing them, so a reader is not invited to trust the page
   character-for-character. Cheap, and honest about what is maintained.
3. **Delete the block** and let `schema/index.md`'s descriptor table be the
   only descriptor reference. The table is the readable form and is current;
   the question is whether anything is actually lost, which wants a look at
   what the exact declarations are used for.

Option 1 is the one that matches how this repo has treated every other
documentation claim it cares about — the pattern is already there in
`test/doc_examples/` and in `test/shipped_header/`, both of which exist
because a hand-maintained copy went stale unnoticed.
