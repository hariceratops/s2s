# A narrow field in big-endian order is written at the wrong end

Found by the architect while designing views, and recorded in the message of
`0e9afce`, which fixed the read side only. Not reproduced since; confirm first.

`struct_cast_be` used to read a `u32` declared `3_B` over `01 02 03` as
`0x01020300`, because the full-width byteswap moved the three bytes read into
the low end of the object up to the top. `0e9afce` reads them at their wire
width. The write side is believed to have the mirrored bug: `stream_cast_be`
writing `0x010203` into a `3_B` field emits `00 01 02` instead of `01 02 03`.

No existing test covers a narrow field in big-endian order on the write path, so
nothing would have caught it.

## To settle
- Reproduce with a `3_B` `u32` through `stream_cast_be`; check `2_B` and `4_B`
  in a `u64` too.
- Fix it where the write path swaps, then round-trip against `struct_cast_be`
  as the oracle, as the read-side test does.
- Whether a value that does not fit the narrow width is an error is a separate
  question, and is not settled here.
