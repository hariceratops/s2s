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

## Notes 2026-08-06
- The criterion "identifying the length field **and** the disagreeing
  dependent" is **partially met**. A contradiction now reports
  `found_contradicting_length` rather than `validation_failure`, so it is no
  longer ambiguous with a value failing its own constraint (owner's call
  2026-08-07, reversing design §5 in part; see design §5.1). Naming the
  disagreeing dependent is still not possible as specified. `cast_error` carries
  one `failed_at`; design §5 considered adding a second name and rejected it
  as out of scope because it changes the read path too. A fan-out
  contradiction is reported at the length field, which is where the write
  stops and therefore where the byte prefix ends. Closing this needs a
  `cast_error` change, i.e. a spec amendment.
- "Overflow checking from 004 applies to the computed length as well" is met
  only for fan-out. A `len_from_fields` result is never serialized — the
  source fields occupy their own declared slots — so there is no width for it
  to overflow.
