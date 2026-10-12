# [feat] an array of records is indexed as sub-views

ELF section headers, MFT entries and directory tables are fixed-width records
repeated a fixed number of times. This slice makes `array_of_records` viewable:
`v["entries"_f][i]` is a sub-view of the *i*-th element, so
`v["entries"_f][i]["flags"_f]` decodes one field of one element and nothing
else.

Creation checks the constrained fields of **every element** of every array of
records, in wire order, because validity of the record is a property of all of
it. The cost is bounded by the number of constrained fields, not the number of
fields.

The index is **not checked**. `[i]` with `i >= N` is a documented precondition
violation, like `std::array::operator[]` and `std::span::operator[]`. Because
trailing bytes past the record are permitted, an out-of-range index can land
inside the span and silently return garbage rather than crash; the docs mark it
as an unchecked spot (082). Once the library's baseline is C++26 this is a
natural place for library hardening and a contract `pre` — recorded in the
spec's Future Directions, not part of this slice.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- The fixed-layout concept accepts `array_of_records` whose element schema is
  itself accepted, recursively, including arrays nested in records nested in
  arrays.
- `v["entries"_f][i]` returns a sub-view of element *i*; its fields read as
  `const` values equal to what `struct_cast` produces, at compile time and at
  run time, in both byte orders.
- The record width constant includes `N` times the element width, and every
  element's offset is a compile-time constant given a constant `i`; no table is
  built or allocated at run time.
- Creation checks the constrained fields of every element, in wire order, and
  reports the parser's `cast_error` for a violation in any element.
- A constraint on the array itself (not only on its elements' fields) is
  checked at creation, with the parser's `cast_error` (design §5.2).
- Reading one field of one element decodes only that field.
- An out-of-range index is a documented precondition; the behaviour is not
  tested for, but the absence of a check is stated in the reference entry when
  082 lands.
- Covered in a `ut` suite (compile time and run time) and a GoogleTest case;
  run-time cases join the 074 UBSan configuration.
- `ctest` is green tree-wide; `single_header/s2s.hpp` is regenerated in the
  same commit.
