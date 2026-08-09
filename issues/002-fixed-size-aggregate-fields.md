# [feat] Fixed-size aggregate fields round-trip

Extend the write dispatch to the fixed-size aggregate field kinds, whose
size is known entirely from the schema and needs no cross-field derivation.

Depends on: 001.

## Acceptance Criteria
- `fixed_array_field` (`std::array<T, N>` of trivials) round-trips.
- `c_arr_field` (`T[N]`) round-trips.
- `fixed_string_field` (`fixed_string<N>`) round-trips, including its
  null-terminator sizing (`field_size<fixed<N + 1>>`).
- `c_str_field` (`char[N + 1]`) round-trips.
- Element-wise byteswapping is applied for foreign-endian schemas.
- **Fix the existing `read_foreign_buffer` bug** at
  `include/field_read/read_impl.hpp:72`: the loop reads
  `for(auto& elem: obj) obj = std::byteswap(obj);` — `elem` is declared and
  never used, and it byteswaps the container into itself rather than each
  element. It does not compile for `std::vector<T>` at all. This is latent
  only because no existing test exercises a foreign-endian variable-sized
  buffer; the write path's round-trip tests will hit it immediately.
- A read-side test covering foreign-endian buffers is added, since the absence
  of one is what hid the bug.
- Round-trip verified for both byte orders, in both `test/runtime/` and
  `test/constexpr/`, with new test files registered in `CMakeLists.txt`.
- No heap allocation introduced.

## Review 2026-08-06
- "No heap allocation introduced" was asserted from reading the code, never
  verified. No test, allocator hook or static check exists anywhere in the
  repo. Judged minor; recorded so the criterion is not silently treated as met.
- Deviation from the approved scope: two read-only tests were added to
  `test/runtime/fixed_buffer_fields.cpp`, which the issue said to leave
  untouched. Disclosed in commit 8aa69b5.
