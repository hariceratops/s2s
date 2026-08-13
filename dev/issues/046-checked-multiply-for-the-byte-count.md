# [fix] Compute the byte count with a checked multiply

`include/field_read/read_impl.hpp:60` computes `len_to_read * sizeof(T{}[0])`
for the non-constexpr path with nothing guarding the product. A large element
type and a large length wrap `std::size_t`, producing a byte count *smaller*
than the number of elements the container was just resized for — so the read
succeeds, the tail of the container is never written, and the caller gets a
fully-sized container with uninitialised elements at the end and no error.

This is a correctness bug in its own right, independent of any bound: it needs
no `max_bytes` declared and no hostile input beyond a length large enough to
wrap. It lands first because the bound check in 047 compares against this same
product, and building that check on a product that can silently wrap would make
the bound unsound in exactly the case it exists to catch.

Spec: `dev/specs/unbounded-resize-from-wire-length.md`.

## Acceptance Criteria
- `count * sizeof(element)` is computed through a checked or saturating
  operation; a product that would wrap `std::size_t` is detected rather than
  silently truncated.
- A length whose byte count would wrap fails with a `cast_error` instead of
  producing a container with an uninitialised tail.
- The product is computed once and reachable by the bound check 047 adds,
  rather than recomputed there.
- Covered at compile time and at run time, per the project's three-way
  `add_ut_test` build; `constexpr_memstream` is enough to drive it.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries.
