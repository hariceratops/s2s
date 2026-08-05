# [feat] Non-invertible lengths and shared-length fan-out

Handle the length forms that cannot be derived. `len_from_fields` /
`size_from_fields` computes a size via an arbitrary user callable over N named
fields, which has no general inverse — so those source fields stay writable
and the write path can only verify consistency, not repair it. Separately,
when several variable-sized fields share one length field, they must all agree.

Depends on: 004.

## Acceptance Criteria
- A schema using `len_from_fields<callable, ids>` round-trips when the stored
  source fields are consistent with the actual container size.
- When the callable evaluated over the stored source fields disagrees with the
  container's real size, the write fails with a validation-style `cast_error`
  naming the field. The write does not silently emit either value.
- The source fields feeding a `len_from_fields` callable remain assignable
  through the non-const `operator[]` — they are ordinary data, and 005's
  read-only constraint must not capture them.
- Shared-length fan-out: when N ≥ 2 variable-sized fields derive their size
  from the same length field, the write derives the length from one dependent
  and verifies every other dependent agrees.
- A contradiction among fan-out dependents fails the write with a `cast_error`
  identifying the length field and the disagreeing dependent.
- Overflow checking from 004 applies to the computed length as well.
- Consistency and contradiction cases covered in `test/runtime/`; round-trip
  also covered in `test/constexpr/`.
