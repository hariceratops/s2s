# The size axis

The size axis answers one question: how many bytes does this field occupy on
the wire? That is not the same question as how large its C++ type is. A field
whose type is `u32` can be declared to occupy two bytes, and reading it will
produce a `u32` holding a value that arrived as two bytes.

A size is a **value**, not a type, and it is written directly as one of five
forms. There is no wrapper around it.

| Form | How the width is determined |
|---|---|
| `N_B` | exactly `N` bytes, fixed at compile time |
| `len_from_field<"id">` | the value currently held by the field named `id` |
| `size_from_fields<f, "a", "b">` | a callable applied to the named sibling fields |
| `len_from_fields<f, "a", "b">` | an alias for `size_from_fields` |
| `size_dont_care` | whatever the field's own nested schema occupies |

**`N_B`** is the only form a `basic_field` accepts, and it is constrained:
`field_fits_to_underlying_type` requires `N <= sizeof(T)`. Declaring a
`basic_field<"x", u16, 4_B>` is a compile error, because four bytes off the
wire cannot be delivered into a two-byte type.

**A `basic_field` that omits its size gets `sizeof(T)`**, which is what most
fields want:

```cpp
s2s::basic_field<"version", u16>          // two bytes
s2s::basic_field<"version", u16, 2_B>     // the same field, spelled out
s2s::basic_field<"truncated", u32, 2_B>   // two bytes on the wire, u32 in the struct
```

The size and the constraint are an unordered pair, so either may be given, in
either order, or neither. An entry that is neither a size nor a constraint is
rejected by a named concept that says what was expected, and giving two of the
same kind is a `static_assert`.

**`len_from_field<"id">`** takes the width from another field's value — the
length-prefix pattern. The named field must appear earlier in the schema, since
reading is strictly left to right and the length has to be known before the
field it sizes.

**`size_from_fields<f, "a", "b">`** covers everything a single field
lookup cannot express: a length in units other than bytes, a total minus a
header, a width assembled from two fields. The callable and its inputs are
described in [Computed values](computed-values.md); `len_from_fields` is the
same template under a name that reads better at a length.

**`size_dont_care`** is what the record descriptors use. It is not zero and it
is not "unknown" — it means the width is whatever the nested schema works out
to, so there is nothing to declare. It is never written by hand.

## Invertible sizes, and why the write path cares

The distinction that matters on the write path is whether a size can be run
backwards.

`len_from_field<"n">` can. If the container holds four elements then `n` is
four, so the library **derives** `n` from the data and makes assigning to it a
compile error. The two can never disagree because only one of them is data
anyone supplies.

`size_from_fields<f, ...>` cannot. `f` is an arbitrary callable; knowing its
output does not reveal its inputs. So its source fields stay assignable and the
library **verifies** instead: it applies `f` at write time and fails with
`found_contradicting_length` if the answer disagrees with the container's real
size.

Reading is unaffected by the distinction — both forms simply produce a width.
[Writing](../writing.md) covers the consequences in full.

## `size_choices` is not currently declarable

`size_choices` exists to give a union alternative its own width, and the
library's internal union machinery uses it. It cannot currently be written into
a schema: `is_selectable_size_v` reads the wrong trait, so the concept that
would admit a `size_choices` rejects it and admits fixed sizes instead. See
`dev/issues/027-is-selectable-size-trait-reads-wrong-trait.md`. It is listed
here for completeness and should be treated as unavailable until that is fixed.
