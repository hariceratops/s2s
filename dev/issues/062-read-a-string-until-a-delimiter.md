# [feat] read a string until a delimiter byte

`field_size.hpp` sorts sizes into two categories, and both resolve to a number
before any byte leaves the stream: `is_fixed_size` for `byte_count` (line 144)
and `is_variable_size` for `field_accessor` and `size_from_fields_t` (lines 157
and 162). `deduce_field_size` turns either into a `std::size_t`
(`field_size_deduce.hpp:18-37`), and the buffer reader hands that count straight
to `read_impl` (`field_reader.hpp:38-53`), which resizes by it and pulls exactly
that many bytes.

A delimited field has no count to hand over. Its length is what the read
produces, not what the read is told. This slice adds the spelling
`s2s::until<u8{0}>`, the third size category it belongs to, and the read itself,
on `str_field` only. `vec_field` follows in 063.

The read is incremental and per-byte. That is a deliberate cost: every delimited
field in the format scan is tens of bytes, and the alternative changes the
stream concept. The spec's Technical Approach records `read_until` as the
identified fast primitive if that ever stops being true; do not build it here.

The allocation bound stops being defensive and becomes the termination
condition. `max_bytes<N>` if declared, otherwise `default_max_bytes`
(`field_size.hpp:124-136`), is the only thing that ends a read whose delimiter
never arrives. The bound counts the **value**, not the wire, so `max_bytes<79>`
admits a 79-byte value followed by its delimiter.

Three outcomes, and the point of the third is that it is not the second:
the delimiter is found; the stream runs dry first, which `buffer_exhaustion`
already describes correctly; or the bound is reached first, which is a new
`delimiter_not_found`. Truncated and corrupt are different facts about a file.

**This slice is larger than its title suggests, and two of the additions are
hard errors rather than missing features.** A delimited size matches none of
`extract_length_dependencies`' four specializations
(`field_list_metadata.hpp`), and that template has no primary definition on
purpose — so the first schema declaring an `until<>` field fails to compile at
`struct_field_list`, before any reader runs, with an incomplete-type diagnostic
pointing at metadata extraction rather than at anything the author wrote.
`field_like`'s disjunction in `field_traits.hpp` does the same one layer up. A
third gate sits ahead of both: option-pack entries are classified per element,
so without an arm in `size_option_like` (`field_options.hpp`) the spelling is
not a size, not a constraint and not a bound — it is an unrecognised entry, and
the widened clause on `str_field` is never even reached. All three belong here;
without them nothing in this slice can be demonstrated.

Design: `dev/design/the-size-axis-cannot-say-read-until-a-byte.md`, §1 and §2.
Spec: `dev/specs/the-size-axis-cannot-say-read-until-a-byte.md`.

## Acceptance Criteria
- `s2s::str_field<"name", s2s::until<u8{0}>>` declares a field that reads up to
  and including the first `0` byte.
- The delimiter is consumed from the stream and is **not** stored in the value —
  the accounting `c_str_field` already does with its `N + 1`.
- `until<d>` rejects a multi-byte or non-byte delimiter through a named concept
  (`delimiter_value_like`); the rejection's test entry is 064's.
- The size form does not satisfy `variable_size_like`, and **no**
  `deduce_field_size` specialization is added for it — its absence is the
  enforcement, since any path reaching it then fails on an incomplete type
  rather than silently reading nothing.
- `size_option_like` gains an arm for the delimited size, so `until<>` is
  classified as a size in an option pack. `str_field<"kw", until<u8{0}>, 4_B>`
  then trips the existing "at most one size option" assertion with no new rule.
- `str_field`'s requires-clause widens through a single new concept covering
  both the variable and delimited categories, admitting nothing it previously
  rejected.
- `extract_length_dependencies` gains a specialization for a delimited size,
  answering "no sibling dependencies" — the same answer a fixed size gives.
- `field_like`'s disjunction in `field_traits.hpp` gains the delimited field
  trait.
- A delimiter in first position yields an empty value, consuming one byte, and is
  not an error. ELF defines string-table index 0 as exactly this.
- The stream running dry before the delimiter reports `buffer_exhaustion`.
- The bound being reached before the delimiter reports a new
  `delimiter_not_found`, appended to `error_reason` (`cast_error.hpp:9`) rather
  than inserted, keeping existing values stable.
- `max_bytes<79>` accepts a 79-byte value plus its delimiter and rejects an
  80-byte one; a field declaring no bound uses `default_max_bytes`.
- A delimiter at or above `0x80` works — the read compares bytes, not signed
  chars.
- No change to `stream_traits.hpp` — no operation is added to the stream concept,
  and every existing custom stream compiles untouched.
- The read loop is one template serving both container types and both stream
  kinds, so 063 adds nothing to it.
- Covered in both forms per the project's standing override: a `ut` suite against
  the constexpr stream and a GoogleTest case against a real `ifstream`, as a
  `test/schema/` pair on that directory's `<feature>_read.cpp` / `_ct.cpp`
  convention.
- **Every bound-related `ut` case declares a small `max_bytes`.** The
  `delimiter_not_found` path at the *default* bound reads 16 MiB one byte at a
  time, which under constant evaluation exceeds gcc's `-fconstexpr-ops-limit`
  and `-fconstexpr-loop-limit`. The default-bound path is exercised in the
  GoogleTest suite only, and that asymmetry is deliberate rather than an
  oversight in coverage.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.

## Review 2026-10-10
- `include/field_read/read_impl.hpp:119,121`: trailing comments `// buffer_exhaustion` and `// consumed, not stored` only restate the line; remove them.
