# [fix] is_selectable_size_v reads is_fixed_size, so size_choices is unusable

`include/field_size/field_size.hpp:133` defines

```cpp
template <typename T>
inline constexpr bool is_selectable_size_v = is_fixed_size<T>::res;
```

It should read `is_selectable_size<T>::res`. The variable template reports the
exact opposite of the trait it is named after: `false` for a `size_choices`
type, `true` for a plain `fixed` size. Verified against the shipped single
header:

```
is_selectable_size_v<size_choices> = 0 (trait says 1)
is_selectable_size_v<fixed<4>>     = 1 (trait says 0)
```

The consequences run through the concepts built on it. `selectable_size_like`
matches fixed sizes and rejects `size_choices`; `is_size_like` is
`fixed_size_like || variable_size_like || selectable_size_like`, so it never
accepts a `size_choices` either. A user cannot declare a field whose size is a
`size_choices`, which is the one thing that type exists for.

The library's own union-size machinery is unaffected — it builds
`size_choices_from_type_conditions` and consumes the resulting types directly
rather than going through the concept — which is why the suite stays green and
nothing in `test/` covers `size_choices` as a user-written size.

Out of scope for the documentation restructure (PRD Non-Goals: defects found in
code are filed separately). Issue 015 documents the size axis with `size_choices`
marked as not currently declarable and points here.

## Acceptance Criteria
- `is_selectable_size_v<T>` reads `is_selectable_size<T>::res`.
- A field declared with a `field_size<size_choices<...>>` satisfies `is_size_like`
  and compiles as part of a `struct_field_list`.
- A test covers reading and writing such a field, under both `test/runtime/` and
  `test/constexpr/`.
- The fix is checked against `selectable_size_like`'s other uses, since every one
  of them has been silently matching fixed sizes.
- Issue 015's documentation is updated to drop the "not currently declarable"
  note once this lands.
