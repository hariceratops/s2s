# [feat] a nested record reads as a sub-view

Fixed-layout formats nest: an ELF section header inside a table, a pcap record
inside a file, a header struct inside a packet. If reading a `struct_field`
decoded the whole nested record, laziness would stop at the first level of
nesting. A `struct_field` therefore returns a **sub-view**, so that
`v["hdr"_f]["len"_f]` decodes exactly one field and nothing around it.

A sub-view is a byte pointer plus an offset folded in at compile time. It never
becomes a typed record pointer, which keeps the alignment rule intact at every
depth. A caller who does want the whole nested record can `struct_cast` over
the sub-view's bytes; that is a use of existing machinery, not a new feature.

Offsets recurse: the prefix sum descends into nested records, and the record's
width — the public constant from 075 — is the sum over the nesting. Constraints
inside nested records are evaluated at creation, in wire order, at every depth;
a violation reports the same `cast_error` the parser does, including the
outermost-field limitation.

Arrays of records are 079.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- The fixed-layout concept accepts a `struct_field` whose nested schema is
  itself accepted, recursively.
- `view["hdr"_f]` returns a sub-view; `view["hdr"_f]["len"_f]` returns the
  field's `const` value, equal to the one `struct_cast` produces, at compile
  time and at run time, in both byte orders.
- Reading a field of a sub-view decodes only that field; no other field of the
  nested record, and no field of the enclosing record, is decoded.
- The record width constant includes nested records; every offset through
  nesting is a compile-time constant.
- Constrained fields inside nested records are checked at creation with the
  parser's `cast_error` (same `failure_reason` and `failed_at`).
- A constraint on the nested record itself (not only on its fields) is checked
  at creation, with the parser's `cast_error` (design §5.2).
- A sub-view is copyable and non-owning; no typed pointer into the buffer is
  formed at any depth.
- Covered in a `ut` suite (compile time and run time) and a GoogleTest case;
  run-time cases join the 074 UBSan configuration.
- `ctest` is green tree-wide; `single_header/s2s.hpp` is regenerated in the
  same commit.
