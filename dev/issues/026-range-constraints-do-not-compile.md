# [fix] range and both range constraints do not compile

`range`'s constructor runs `static_assert(value1 < value2, ...)` against its own
function parameters, which are not constant expressions. Constructing a `range`
is therefore a hard compile error, and because `is_in_open_range` and
`is_in_closed_range` both take `range<T>` arguments, all three constructs are
unusable as shipped.

Found while writing issue 018, by compiling `constexpr s2s::range<int> r{1, 5};`
against the shipped single header:

```
s2s.hpp:1118:26: error: non-constant condition for static assertion
 1118 |     static_assert(value1 < value2, "Range start must be less than range end");
s2s.hpp:1118:19: error: 'value1' is not a constant expression
```

Nothing in `test/` exercises `range`, `is_in_open_range`, or `is_in_closed_range`
— the only matches are inside the generated `determinism_*.hpp` amalgam
artifacts — which is why three broken public constructs shipped unnoticed.

Out of scope for the documentation restructure (PRD Non-Goals: defects found in
code are filed separately). Issue 018 documents the constraint DSL minus these
three and points here.

## Acceptance Criteria
- `constexpr s2s::range<int> r{1, 5};` compiles.
- The ordering precondition is still enforced at compile time where the values
  are known at compile time. A `static_assert` on function parameters is not the
  mechanism; the check belongs wherever the range is used as a non-type template
  argument, or expressed so an invalid range is not representable.
- `is_in_open_range` and `is_in_closed_range` are usable as field constraints on
  a real schema, in both read and write directions.
- Tests cover all three under `test/runtime/` and `test/constexpr/`, including
  the boundary difference that distinguishes open from closed.
- Issue 018's documentation is updated to drop the "not currently usable" note
  once this lands.
