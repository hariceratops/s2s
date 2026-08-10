# [feat] Derived fields become compile-time read-only

Make it impossible to desynchronize an invertible dependent field from the
data it describes, by removing the ability to assign to it at all. The
non-const `operator[]` on `struct_field_list_impl` gains a `requires` term
that excludes derived fields; the const overload stays unconstrained so a
parsed value can still be read.

This is a **deliberate breaking change** to existing read-side usage:
`s["len"] = 5` on a parsed struct will stop compiling.

Depends on: 004. Extended by 009 for `type_switch` union discriminants.

## Acceptance Criteria
- A compile-time predicate identifies derived fields by scanning
  `list_metadata` for `field_accessor` references — initially, fields targeted
  by a `len_from_field<"id">`.
- The non-const `operator[]` overload in `include/field_list/field_list.hpp`
  gains an additional `requires` term rejecting derived fields. Enforcement is
  a constraint, not a runtime check or a `static_assert` in the body.
- The const `operator[]` overload continues to compile for derived fields, so
  reading a parsed length still works.
- `struct_cast` is unaffected: the read path populates fields via
  `static_cast<fields&>(field_list).value`, not through `operator[]`. An
  existing read test covering a `len_from_field` schema still passes unchanged.
- A compile-fail test demonstrates that assigning to a derived field does not
  compile, and that the const read of the same field does.
- Fields that are *not* referenced as a length source remain assignable.
- The diagnostic for an attempted assignment is readable — a short
  assign-to-const error, not a wall of unsatisfied-constraint noise. Because
  the const overload remains viable, overload resolution selects it and the
  compiler reports assignment to a const reference; the message will not
  itself say "derived", and that is accepted. Readable template diagnostics
  are why this project hand-rolls its vocabulary types, so a constraint dump
  here would still be a defect.
