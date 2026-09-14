# [feat] reject writing a run containing the sentinel value

This is the only failure mode in the feature that would otherwise produce a
wrong answer instead of an error, which is why it is a slice rather than a
footnote on 071 — the same shape as the companion's rejection of a value
containing its own delimiter.

If the caller's vector holds an element whose named field happens to equal
the sentinel — a mid-run zero-size `sub_block`, say — 071 writes it whole,
then synthesises a terminator after it, as it would for any other element.
The file reads back with the run ending early: every element after the match
is silently lost, and nothing anywhere raises an error. The write has to
fail instead.

No existing part of the library has a comparable content-based write
rejection outside the companion's delimiter case, for the same reason: every
other size form carries its length out of band, so a value's content never
collides with its own framing. Here, an element *is* both data and a
candidate terminator, and only a check on its content stands between a
caller and a corrupt file.

The rejection gets its own `error_reason` rather than folding into
`validation_failure` — the schema author has not violated a constraint they
wrote; they have hit a rule of the termination form itself, appended to the
enum alongside `sentinel_not_found` from 069, keeping existing values stable.

Spec: `dev/specs/a-record-sequence-cannot-end-at-a-sentinel.md`.

## Acceptance Criteria
- Writing a vector containing an element whose named field equals the
  sentinel value fails, with a new appended `error_reason` (e.g.
  `found_sentinel_in_sequence`), distinct from `validation_failure` and
  `sentinel_not_found`.
- `failed_at` names the offending field.
- The check runs per element on write; failure happens at the offending
  element with no rollback — earlier elements may already be on the stream,
  matching every other write-side rejection in the library. The whole vector
  is not pre-validated before anything is emitted.
- A match in any position — first, last, or middle of the run — is rejected;
  there is no "it's already at the end, treat it as already terminated"
  exemption.
- The check costs nothing on the read path and nothing for any other size
  form or termination kind.
- The rule is stated in the docs alongside the form itself, in 073, not left
  to be discovered by hitting it.
- Covered in both constexpr-stream and runtime forms.
- `ctest` is green tree-wide, including the `*_compile_time` and `*_coverage`
  entries; `single_header/s2s.hpp` is regenerated in the same commit.
