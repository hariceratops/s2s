# [fix] c_arr_field and c_str_field cannot be read during constant evaluation

`include/field_read/read_impl.hpp:21`, the constexpr-stream overload of
`read_native_impl`, ends with

```cpp
obj = std::bit_cast<T>(as_byte_buffer_rep);
```

`std::bit_cast` returns its result by value, and a C array cannot be returned
by value. For `T = char[4]` or `T = u16[2]` — the storage types behind
`c_str_field<id, N>` and `c_arr_field<id, T, N>` — this does not compile:

```
error: no matching function for call to 'bit_cast<char [4]>(std::array<char, 4>&)'
```

The write direction is unaffected. `as_byte_buffer` bit_casts *from* the
object, and a C array is a valid source. Confirmed by probe: a
`c_arr_field<"c", u16, 2>` schema writes at compile time with no diagnostic and
fails to read with three errors.

`docs/compile-time.md:33-34` lists both descriptors under "Fine at compile
time", opposite the two allocating descriptors they exist to replace. The
documented claim was false for half the round trip.

Nothing caught it because no test read either descriptor through a constexpr
stream. The runtime path uses `byte_addressof` and is fine, and the write-side
constexpr tests only ever used `fixed_array_field` and `fixed_string_field`.

Found while writing 030's read-side compile-time coverage, which requires a
case for each of the four const-sized descriptors.

## Acceptance Criteria
- A `c_arr_field` and a `c_str_field` can each be read through a constexpr
  stream.
- The runtime path is unchanged.
- Compile-time coverage exists for both descriptors on the read side.

## Notes 2026-08-12

Fixed in 030 rather than deferred — it blocked that issue's done-bar, and the
docs already promised the behaviour.

`read_native_impl` now branches on `std::is_array_v<T>`: the byte buffer goes
to the equivalent `std::array` and the elements are assigned individually.
Non-array types keep the original `bit_cast`. Coverage is
`test/fields/fixed_buffer_read_ct.cpp`, which reads all four const-sized
descriptors at compile time.
