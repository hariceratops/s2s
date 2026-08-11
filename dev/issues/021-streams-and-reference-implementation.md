# [chore] Streams, with an in-doc reference implementation

"Pluggable interfaces working with custom streams" is a README headline whose
only documentation is a pointer into the test tree, and `UserGuide.md`'s table
of contents promises a "Using custom stream" section that does not exist. A user
writing a stream type today has to reverse-engineer the requirements from
`include/stream/stream_traits.hpp`.

## Acceptance Criteria
- All seven stream concepts are documented: `read_trait`, `write_trait`,
  `constexpr_read_trait`, `constexpr_write_trait`, `input_stream_like`,
  `output_stream_like`, `constexpr_stream`.
- Each concept's required operations are listed explicitly — what a type must
  provide to satisfy it, not just the concept's name.
- A real reference implementation appears in the documentation itself, rather
  than the current pointer at `test/utils/constexpr_memstream.hpp`.
- It is stated which concepts a runtime stream must satisfy versus a constexpr
  one, and that stream-touching code in this project is written against both.
- The reference implementation is backed by a registered CTest target under
  `test/single_header/` that both compiles it and casts through it.
