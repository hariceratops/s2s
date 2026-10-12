# [feat] a view reads fixed arrays and fixed strings

075 gave a view its scalar fields. Real fixed-layout records also carry byte
arrays, tags and fixed-width text — a pcap magic, an ELF identification block,
a four-character chunk id. This slice extends the accepted subset, and the
fixed-layout concept with it, to the fixed-width aggregate field kinds.

Two of them cannot be returned the way the parser stores them. `c_arr_field` is
a `T[N]` and `c_str_field` a `char[N + 1]`, and a function cannot return a C
array by value. The view returns the equivalent `std::array<T, N>` /
`std::array<char, N + 1>` with the same elements; the type difference from the
parsed struct is deliberate and documented (082). Rejecting those schemas would
turn down a schema for how a field is spelled, not for its layout.

The return is a `const` value specifically so that `view["arr"_f] = {...}`
fails to compile — on a class-type value an assignment would otherwise compile
and silently write to a temporary.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- The fixed-layout concept additionally accepts `fixed_array_field`,
  `c_arr_field`, `c_str_field` and `fixed_string_field`, composed from named
  sub-concepts.
- `fixed_array_field` of trivials reads as the whole `std::array<T, N>`;
  `fixed_string_field` reads as `fixed_string<N>`.
- `c_arr_field` reads as `std::array<T, N>` and `c_str_field` as
  `std::array<char, N + 1>`. Equivalence with `struct_cast` is checked
  elementwise for these two.
- Foreign byte order swaps array elements as the parser does
  (`read_foreign_buffer`); a `fixed_string` is not swapped.
- Each whole-object read is a copy into a local of the result type; no element
  pointer into the buffer is formed.
- `fixed_string::to_sv` is callable on the `const fixed_string` a view returns
  (it is a non-const member today; design F8).
- Assigning through a view accessor (`view["arr"_f] = {...}`) does not compile;
  a `must_not_compile` case registered with `add_rejected_case_matching`
  proves it fails for that reason.
- Equivalence with `struct_cast_le`/`_be` holds at compile time and at run
  time, in a `ut` suite and a GoogleTest case; run-time cases join the 074
  UBSan configuration.
- `ctest` is green tree-wide; `single_header/s2s.hpp` is regenerated in the
  same commit.
