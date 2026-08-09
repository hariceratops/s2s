# User Guide [Work in Progress]
Struct
Reading from stream
    Trivial
    Array of trivials
    Array of records
    Length prefixed vector of trivials
    Length prefixed vector of records
    Const sized strings
    Length prefixed strings
    Optionals
    Unions
    Magic strings
    Magic numbers
Writing to stream
    A worked example
    Derived fields are read-only
    What is checked at write time
    Write-once, fail-fast
Using custom stream
Validating field members
Error Handling


## The struct
```cpp
template <typename... fields>
struct struct_field_list { /* implementation */ };
```
A struct_field_list datatype is a meta-struct with a map-like interface to access
a field member for read or write operation. The operator[] when provided with "<field_name>"_f
as key, returns a reference or const reference to the member with name <field_name>

Accessing field not present in the struct_field_list(the "map") will result
in compilation error for field lookup failure, since a check is enforced via concepts 

```cpp
template <typename field_accessor, typename field_lookup = /* field_lookup metafunction */>
    requires /* field_lookup success */
auto& operator[](field_accessor);

template <typename field_accessor, typename field_lookup = /* field_lookup metafunction */>
    requires /* field_lookup success */
const auto& operator[](field_accessor);
```


## Field Descriptors
```cpp
using our_struct = 
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;
```
Library provides users a way to describe the fields contained in the struct_field_list
Each descriptor is a variadic template, describing the name, type, size, constraint on 
value along with type deduction or presence deduction guides if any

Available descriptors are: basic_fields, fixed_array, fixed_string, 
array_of_records, vec_field, str_field, vector_of_records,
magic_string, magic_number, magic_byte_array, union_field and maybe


template <fixed_string id, integral T, fixed_size_like size_type, auto constraint_on_value = no_constraint<T>{}>
  requires field_fits_to_underlying_type<size_type, T>
using basic_field = field<id, T, size_type, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using fixed_array_field = field<id, std::array<T, N>, field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, field_list_like T, std::size_t N, auto constraint_on_value = no_constraint<std::array<T, N>>{}>
using array_of_records = field<id, std::array<T, N>, field_size<size_dont_care>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<fixed_string<N>>{}>
using fixed_string_field = field<id, fixed_string<N>, field_size<fixed<N + 1>>, constraint_on_value>;

template <fixed_string id, field_containable T, std::size_t N, auto constraint_on_value = no_constraint<T[N]>{}>
using c_arr_field = field<id, T[N], field_size<fixed<N * sizeof(T)>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto constraint_on_value = no_constraint<char[N + 1]>{}>
using c_str_field = field<id, char[N + 1], field_size<fixed<N * sizeof(char) + 1>>, constraint_on_value>;

template <fixed_string id, std::size_t N, auto expected>
using magic_byte_array = field<id, std::array<unsigned char, N>, field_size<fixed<N>>, eq{expected}>;

template <fixed_string id, fixed_string expected>
using magic_string = field<id, fixed_string<expected.size()>, field_size<fixed<expected.size() + 1>>, eq{expected}>;

template <fixed_string id, integral T, fixed_size_like size, auto expected>
using magic_number = field<id, T, size, eq{expected}>;

// todo get vector length in bytes instead of size to read additional overload
// todo how user can provide user defined vector impl or allocator
template <fixed_string id, typename T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vec_field = field<id, std::vector<T>, size, constraint_on_value>;

template <fixed_string id, field_list_like T, variable_size_like size, auto constraint_on_value = no_constraint<std::vector<T>>{}>
using vector_of_records = field<id, std::vector<T>, size, constraint_on_value>;

// todo check if this will work for all char types like wstring
template <fixed_string id, variable_size_like size, auto constraint_on_value = no_constraint<std::string>{}>
using str_field = field<id, std::string, size, constraint_on_value>;

template <fixed_string id, field_list_like T>
using struct_field = field<id, T, field_size<size_dont_care>, no_constraint<T>{}>;


## Cast
```cpp
template <struct_field_list_like T, stream_like S>
auto struct_cast_le(S& stream) -> std::expected<T, cast_error>;

template <struct_field_list_like T, stream_like S>
auto struct_cast_be(S& stream) -> std::expected<T, cast_error>;
```
The APIs struct_cast_xx reads from a stream into struct_field_list, when 
provided a stream as a runtime argument and a struct-schema as a template argument. 
The xx is either le or be denoting byteorder of all the struct members.
The APIs return std::expected which either contains a struct_field_list or read_error


## Writing to stream
```cpp
template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto struct_write_le(S& stream, const T& obj) -> std::expected<void, cast_error>;

template <struct_field_list_like T, output_stream_like S>
[[nodiscard]] auto struct_write_be(S& stream, const T& obj) -> std::expected<void, cast_error>;
```
The APIs struct_write_xx serialize a struct_field_list to a stream. They mirror
struct_cast_xx: the same schema drives both directions, the xx suffix picks the
byte order of every member, and failures come back as the same cast_error.
There is nothing to return on success, so the expected holds void.

The struct is written strictly left to right, one field at a time, in
declaration order and at every nesting level.

### A worked example
```cpp
#include "s2s.hpp"
#include <sstream>
#include <vector>

using namespace s2s_literals;
using u16 = unsigned short;
using u32 = unsigned int;

using our_struct =
  s2s::struct_field_list<
    s2s::magic_string<"magic", "S2S">,
    s2s::basic_field<"count", u32, s2s::field_size<s2s::fixed<4>>>,
    s2s::vec_field<"data", u16, s2s::field_size<s2s::len_from_field<"count">>>
  >;

auto main() -> int {
  our_struct obj{};
  obj["magic"_f] = s2s::fixed_string<3>("S2S");
  obj["data"_f] = std::vector<u16>{0x1122, 0x3344};
  // Note: "count" is never assigned. It is derived from data.size().

  std::stringstream le(std::ios::in | std::ios::out | std::ios::binary);
  if(auto res = s2s::struct_write_le<our_struct>(le, obj); !res)
    return 1;

  std::stringstream be(std::ios::in | std::ios::out | std::ios::binary);
  if(auto res = s2s::struct_write_be<our_struct>(be, obj); !res)
    return 1;

  // Read either one back with the matching byte order.
  auto back = s2s::struct_cast_be<our_struct>(be);
  return back && (*back)["count"_f] == 2 ? 0 : 1;
}
```

### Derived fields are read-only
Some fields are not data you supply — they are consequences of data you
supply. Writing them from the struct would let the two disagree, producing a
stream that does not read back as what you handed in. So the library derives
them, ignores whatever the struct holds, and makes assigning to them a
compile error.

Two kinds of field are derived:

| Field | Derived from |
|---|---|
| the target of a `len_from_field<"n">` size | the container's `size()` |
| the `match_field` discriminant of a `type_switch` variance | the `match_case` value of the alternative currently held |

Both are invertible, which is what makes deriving them possible at all.

```cpp
our_struct obj{};
obj["data"_f] = std::vector<u16>{1, 2, 3};
obj["count"_f] = 3;   // does not compile
```

The non-const `operator[]` returns a const reference for a derived field, so
the diagnostic is a plain assignment-to-read-only error. Reading still works
through either subscript:

```cpp
const auto n = obj["count"_f];                 // compiles
const auto m = std::as_const(obj)["count"_f];  // compiles
```

Note what those reads give you: **the stored slot, not the derived value.**
Derivation happens during the write, against the struct you pass as `const`,
and the result is never written back. On a struct you built yourself the slot
is whatever it was default-constructed to:

```cpp
our_struct obj{};
obj["data"_f] = std::vector<u16>{1, 2, 3};
assert(obj["count"_f] == 0);   // not 3 — nothing has been written yet
```

On a struct that came from `struct_cast` the slot holds what was parsed off
the wire, which is the value you usually want. If you need the length before
writing, ask the container: `obj["data"_f].size()`.

**This is a breaking change to existing read-side code.** If you previously
parsed a struct and then adjusted a length field in place:

```cpp
auto parsed = *s2s::struct_cast_le<our_struct>(stream);
parsed["count"_f] = 5;   // used to compile, no longer does
```

replace it by assigning the container instead. The length follows:

```cpp
parsed["data"_f] = std::vector<u16>(5);   // "count" becomes 5 on the next write
```

Fields that are *not* derived stay assignable, including ones that look
similar: the sources feeding a `len_from_fields<callable, ...>` callable, the
siblings feeding a `parse_if` predicate, the fields feeding an
`if_else_ladder` branch, and a length whose only container sits inside a
`maybe` or a union alternative. None of those can be inverted, so they remain
yours to set — and are verified rather than derived.

### What is checked at write time
Everything the library can check without a second pass is checked *before*
the offending field's first byte is emitted.

| Inconsistency | `failure_reason` | `failed_at` |
|---|---|---|
| `constraint_checker` rejects the value (e.g. a wrong magic value) | `validation_failure` | that field |
| a `parse_if` predicate disagrees with the optional's `has_value()` | `validation_failure` | the optional field |
| an `if_else_ladder` or computed-switch selects an alternative other than the one held | `validation_failure` | the union field |
| no ladder branch and no `match_case` matches at all | `type_deduction_failure` | the union field |
| a derived or verified length does not fit its declared width | `validation_failure` | the length field |
| a `len_from_fields` callable disagrees with the container's real size | `found_contradicting_length` | the container field |
| two containers sharing one length field imply different lengths | `found_contradicting_length` | the length field |
| a length inside a `maybe` or union alternative disagrees while that producer is active | `found_contradicting_length` | the length field |
| the underlying stream refuses the write | `buffer_exhaustion` | the field being written |

For a violation inside a nested record, `failed_at` names the outermost record
field rather than the inner one, matching how the read path reports nested
failures.

### Write-once, fail-fast
There is no rollback and no size query. Bytes handed to your stream are gone,
so the unit of atomicity is one field rather than the whole write:

- every derivation, verification and constraint check for a field runs before
  its first byte is emitted;
- a failure attributed to field `k` therefore leaves fields `0..k-1` in the
  stream and nothing of `k` — a well-defined prefix.

The one exception is a stream that fails part way through a single field: how
much it consumed is the stream's business, and the library reports
`buffer_exhaustion`. Recovering from a partial write is the caller's
responsibility — truncate, discard, or write to a staging buffer first if you
need all-or-nothing.


## Errors
Both directions report failures the same way. `cast_error` carries the reason
and the name of the field it happened at.
```cpp
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure,
  found_contradicting_length
};

struct cast_error {
  error_reason failure_reason;
  std::string_view failed_at;
};
```
A read can fail in three ways: field value validation failure, the input
stream is exhausted, or type deduction failed while reading into a union.

A write can fail in those same three ways plus `found_contradicting_length`,
which means two parts of the struct imply different lengths for the same data
— a cross-field disagreement rather than a value that is wrong on its own
terms. See the table above for which check produces which reason.
