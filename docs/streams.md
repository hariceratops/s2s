# Streams

The library never owns a stream and never creates one. It calls two operations
on whatever it is handed and checks whether the thing still looks healthy
afterwards. That is the whole contract, and satisfying it is how "pluggable
interfaces working with custom streams" is meant to be acted on.

## What a stream must provide

Two concepts gate the public API, and each is a disjunction underneath.

```cpp
template <typename T>
concept input_stream_like =
  (identified_as_constexpr_stream<T> || readable<T>) && convertible_to_bool<T>;

template <typename T>
concept output_stream_like =
  (identified_as_constexpr_stream<T> || writeable<T>) && convertible_to_bool<T>;
```

| Concept | Required expression | Returning |
|---|---|---|
| `read_trait<T>` | `obj.read(char*, std::streamsize)` | `T&` |
| `std_read_trait<T>` | `obj.read(char*, std::streamsize)` | convertible to `std::istream&` |
| `write_trait<T>` | `obj.write(const char*, std::size_t)` | `T&` |
| `std_write_trait<T>` | `obj.write(const char*, std::size_t)` | convertible to `std::ostream&` |
| `readable<T>` | either read form | — |
| `writeable<T>` | either write form | — |
| `convertible_to_bool<T>` | `obj.operator bool()` and `!obj` | `bool` |

The `std_` variants exist so the standard streams work untouched: `std::ifstream`
and `std::fstream` return `std::istream&` from `read`, which is why they satisfy
`std_read_trait` rather than `read_trait`. A hand-written type will normally
return `T&` and satisfy the plain form. Either is accepted.

`convertible_to_bool` is not optional and is easy to miss. The library tests the
stream after every access, so it needs both `operator bool` and `operator!` to
yield `bool`. An `explicit operator bool` satisfies both, because `!obj` is a
contextual conversion.

**Failure is reported by going bad, not by throwing.** A stream that cannot
satisfy a read marks itself unhealthy and returns; the library observes that on
the next `bool` test and reports `buffer_exhaustion` against the field it was
working on. Throwing out of `read` or `write` propagates through the library,
which is not designed to be exception-safe part way through a field.

## A reference implementation

<!-- docs: test/doc_examples/guide_stream_example.cpp -->
```cpp
#include "s2s.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <ios>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u32 = unsigned int;

// A complete stream, satisfying read_trait, write_trait and convertible_to_bool.
// Nothing else is required: no seeking, no exceptions, no std::ios base.
class byte_stream {
  std::vector<char> buffer;
  std::size_t read_pos{0};
  bool ok{true};

public:
  // read_trait: takes a raw destination and a count, returns *this by reference.
  auto read(char* dest, std::streamsize size_to_read) -> byte_stream& {
    const auto n = static_cast<std::size_t>(size_to_read);
    if(read_pos + n > buffer.size()) {
      ok = false;               // report exhaustion by going bad, not by throwing
      return *this;
    }
    std::copy_n(buffer.begin() + static_cast<std::ptrdiff_t>(read_pos), n, dest);
    read_pos += n;
    return *this;
  }

  // write_trait: takes a raw source and a count, returns *this by reference.
  auto write(const char* src, std::size_t size_to_write) -> byte_stream& {
    buffer.insert(buffer.end(), src, src + size_to_write);
    return *this;
  }

  // convertible_to_bool: the library tests the stream after every access.
  explicit operator bool() const { return ok; }
};

// A heartbeat frame, small enough that the stream is the interesting part.
using heartbeat =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x48, 0x42}>,
    s2s::basic_field<"sequence", u32, s2s::field_size<s2s::fixed<4>>>
  >;

static_assert(s2s::input_stream_like<byte_stream>);
static_assert(s2s::output_stream_like<byte_stream>);

auto main() -> int {
  heartbeat beat{};
  beat["marker"_f] = std::array<u8, 2>{0x48, 0x42};
  beat["sequence"_f] = 0xdeadbeefu;

  byte_stream stream;

  const auto round_tripped =
    s2s::stream_cast_be<heartbeat>(stream, beat)
      .and_then([&stream] { return s2s::struct_cast_be<heartbeat>(stream); })
      .transform([](const heartbeat& parsed) {
        return parsed["sequence"_f] == 0xdeadbeefu;
      });

  if(!round_tripped.value_or(false))
    return 1;

  // Reading past the end sets the bad flag, which surfaces as buffer_exhaustion.
  const auto overrun = s2s::struct_cast_be<heartbeat>(stream);
  if(overrun.has_value())
    return 1;

  return overrun.error().failure_reason == s2s::error_reason::buffer_exhaustion ? 0 : 1;
}
```

The `static_assert`s are worth keeping in real code. Failing the concept at the
point of definition produces a readable error; failing it at the call to
`struct_cast` produces a template instantiation trace.

## Constexpr streams

A stream usable during constant evaluation is identified by inheritance, not by
its operations:

```cpp
template <std::size_t N>
class memstream: s2s::constexpr_stream { /* ... */ };
```

`s2s::constexpr_stream` is an empty base struct used as a tag, and
`identified_as_constexpr_stream<T>` tests for it. Deriving from it is what puts
a type on the constexpr path — `input_stream_like` and `output_stream_like`
accept such a type without checking its read and write operations at all.

A constexpr stream works on `std::array<char, N>` rather than `char*`, because
pointer arithmetic over a buffer is not available during constant evaluation:

```cpp
template <std::size_t dest_buffer_size>
constexpr auto read(std::array<char, dest_buffer_size>& dest, std::size_t n) -> memstream&;

template <std::size_t src_buffer_size>
constexpr auto write(const std::array<char, src_buffer_size>& src, std::size_t n) -> memstream&;
```

[Compile time](compile-time.md) carries a complete one, and
`test/utils/constexpr_memstream.hpp` adds the `rewind()` needed to read back
what was just written.

!!! warning "The constexpr traits are not enforced"

    Two concepts, `constexpr_read_trait<T, N>` and `constexpr_write_trait<T, N>`,
    describe exactly those two signatures — but nothing in `include/` refers to
    them. A constexpr stream is admitted on the strength of its base class
    alone, so these two document the expected shape without enforcing it.
    Deriving from `constexpr_stream` with the wrong signatures fails later,
    inside the read or write, rather than at the concept check.
