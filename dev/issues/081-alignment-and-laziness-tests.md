# [test] a view is alignment-safe and lazy, demonstrated rather than asserted

The two properties that justify a view over `struct_cast` — it is safe on a
buffer that is not aligned, and it decodes only what is read — are structural.
Equivalence tests in 075–079 do not prove either: x86 does not trap on a
misaligned typed load, so a regression to `*reinterpret_cast<const T*>` would
still produce correct values there, and a timing test is rejected as flaky. This
slice pins them with tests that fail if either is lost.

Alignment is proven by running misaligned cases under the UBSan configuration
from 074, where a typed misaligned access is a fatal report. Laziness is proven
by mutation: change the bytes of a field that is neither constrained nor read,
and observe that nothing observable changes.

Spec: `dev/specs/reading-two-fields-costs-a-whole-record.md`.

## Acceptance Criteria
- A view over a buffer starting at an **odd address** reads correct values in
  both byte orders (a `char` buffer offset by one into a larger array).
- A schema whose fields sit at **misaligned offsets** — `u8`, then `u32`, then
  `u16` — reads correct values in both byte orders, through nested records and
  array-of-records elements as well as at the top level.
- Both cases are in the run-time suite that the 074 configuration runs, and
  produce no UBSan report there. Reintroducing a typed pointer into the buffer
  would make that run fail; this is checked once by hand and noted in the issue
  file.
- Mutating the bytes of an unconstrained field that is never read changes
  neither creation's result nor any value read, in both byte orders, at the
  top level and at depth.
- Creation and reads allocate nothing (there is no existing
  allocation-counting helper; the design uses a replacement `operator new` in
  the view's own GoogleTest file).
- A dynamic span one byte short fails with `buffer_exhaustion`; a static span
  one byte short does not compile (a `must_not_compile` case), if not already
  covered by 075.
- `ctest` is green tree-wide, and the documented 074 command is green.
