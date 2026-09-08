# [feat] thread byte order as a runtime value through the read path

`struct_cast_impl` carries endianness as a template parameter
(`include/cast/struct_cast_impl.hpp:13`) and hands it to each field's reader as
`reader.template read<endianness>(s)` at line 30. For a schema whose byte order
is a fact about the *file*, that value cannot be known when the template is
instantiated, so the fold has to carry it as a runtime value instead.

No user-visible feature lands here. `struct_cast_le` and `struct_cast_be` keep
their signatures and behaviour exactly; they now collapse to a runtime
`cast_endianness` at the entry point rather than threading a compile-time one
through every layer. This slice exists on its own because it is the riskiest
change in the feature and is independently testable — the whole suite green
under runtime threading is the acceptance condition.

The one non-mechanical edit is the leaf. `read_impl`
(`include/field_read/read_impl.hpp:129-140`) branches
`if constexpr(byte_order == cast_endianness::host) / else if constexpr(... ==
foreign)`, and the foreign arm has no `else`: for a `T` that is neither
`trivial` nor `buffer_like` it falls off the end. That is sound today only
because the arm is never instantiated for such a `T`. Once the outer branch is
a runtime `if`, both arms are instantiated for every `T`, so the foreign side
has to become total. The inner `if constexpr`s stay as they are.

Spec: `dev/specs/byte-order-is-data-not-a-template-parameter.md`.

## Acceptance Criteria
- The outer host/foreign branch in `read_impl` is a runtime `if/else`; the
  inner `if constexpr` discrimination between `trivial`, `buffer_like` and the
  rest is unchanged.
- The foreign arm is total: it compiles and does the right thing for every `T`
  the host arm accepts, with no path that falls off the end.
- `struct_cast_impl`'s fold carries `cast_endianness` as a runtime value rather
  than the `auto endianness` template parameter at line 13.
- `struct_cast_le` and `struct_cast_be` are unchanged in signature and
  behaviour; they resolve to a runtime value at the entry point.
- No new public API and no new schema spelling.
- Every existing read test passes unmodified — no test is edited to accommodate
  this slice.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
