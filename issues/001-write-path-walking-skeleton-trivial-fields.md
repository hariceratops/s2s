# [feat] Write path walking skeleton — trivial fields round-trip

Establish the end-to-end write path for the simplest field kind, so every
later slice has a spine to hang off. This delivers the public
`struct_write_le`/`struct_write_be` entry points, the low-level write
primitives, and a field-kind-dispatched writer driven through the same
field-list fold the read path uses, in declaration order.

## Acceptance Criteria
- `struct_write_le<T>(stream&, const T&) -> cast_result` and
  `struct_write_be<T>(stream&, const T&) -> cast_result` exist in `include/api/`,
  constrained on `field_list_like T` and `output_stream_like stream`, and are
  marked `[[nodiscard]] constexpr`.
- The unused `output_stream` scaffold at the bottom of
  `include/field_read/read_impl.hpp` is **removed, not completed**: it returns
  `rw_result` where `write_trait` requires `-> std::same_as<T&>`, so it cannot
  conform, and the read side has no wrapper class to mirror. It is replaced by
  `write_native_impl` free-function overloads in the write-side headers, one
  constrained on `identified_as_constexpr_stream` and one on `writeable`,
  mirroring `read_native_impl`.
- `output_stream_like` in `include/stream/stream_traits.hpp` is extended to
  admit constexpr streams — today it is `writeable<T> && convertible_to_bool<T>`
  and lacks the `identified_as_constexpr_stream` branch its read-side
  counterpart `input_stream_like` has, which leaves the constexpr write path
  unconstrainable.
- A single public entry point per byte order dispatches internally to the
  constexpr-buffer path or the runtime-stream path, with no visible difference
  to the caller.
- `basic_field` trivials round-trip: populate a struct, write it, read it back
  with `struct_cast_le`/`be`, and compare equal to the original.
- Round-trip holds for both little- and big-endian schemas, including the
  foreign-endian byteswap path.
- Writing proceeds strictly left-to-right in field declaration order.
- Underlying stream-write failure returns
  `std::unexpected(cast_error{error_reason::buffer_exhaustion, ...})`.
- Tests exist in both `test/runtime/` and `test/constexpr/` and are registered
  in the respective `CMakeLists.txt`. Existing read tests are untouched.
- No heap allocation beyond what foreign-endian byteswapping already requires.

## Review 2026-08-06
- "No heap allocation introduced" was asserted from reading the code, never
  verified. No test, allocator hook or static check exists anywhere in the
  repo. Judged minor; recorded so the criterion is not silently treated as met.
