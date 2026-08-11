
# s2s
A declarative binary parser aka serde to convert a stream into meta-struct which has a map like 
interface, and back again.

Implemented as an embedded DSL powered by C++23 TMP.

**Your parser is generated at compile time.** You declare the schema as a type
and the template machinery turns it into straight-line read and write code —
no runtime dispatch over the field list, no reflection, no format interpreter.
This is unconditional. It holds when the stream is a plain `std::ifstream` and
when the schema is full of `std::string` and `std::vector`, and it is what you
get for free by using the library at all.

**Parsing can additionally be performed at compile time.** This is the narrower
claim, and it has two conditions: the stream must be usable during constant
evaluation, and the schema must have no allocating fields. When both hold, the
whole parse — validation included — happens in the compiler and the result is a
`constexpr` value. When they do not, you still get everything in the paragraph
above.

Library is single header and the file "s2s.hpp" from the single_header
folder can be used for direct inclusion into a project

## Features
* Single header
* Parser generated at compile time, unconditionally; parsing itself performed at
  compile time when the stream and schema allow it
* Support for 
    * Trivial
    * Array of trivials 
    * Array of records 
    * Length prefixed vector of trivials
    * Length prefixed vector of records
    * Const sized strings
    * Length prefixed strings
    * Optionals
    * Unions
    * Magic strings
    * Magic numbers
* Writing a struct back to a stream with the same schema, in either byte order
* Lengths and union discriminants derived on write, and read-only at compile
  time so they cannot drift from the data they describe
* Validation of fields in place while reading and while writing
* Compile time endianness handling 
* Pluggable interfaces working with custom streams

## Requirements
s2s currently has a constraint on minimum version of the std to be C++23

The compiler version requirements are 
* gcc 13.1 : x86-64, arm, arm64 gcc 13.1 — runtime use only. Constexpr use of
  `struct_field_list` needs gcc 14 or newer; gcc 13 rejects the `static
  constexpr` metadata tables with "accessing value of 'key' through a glvalue
  in a constant expression".
* clang 19.1.0 : x86-64, armv8-a
* msvc v19.39, VS 17.9 : x64, x86, arm64


## Taste of the API
<!-- docs: test/single_header/readme_read_example.cpp -->
```cpp
#include "s2s.hpp"

#include <print>
#include <sstream>
#include <string>

using namespace s2s_literals;

// Two members: a length field of size 8 and type std::size_t, and a
// length-prefixed string whose length is derived from the "len" field.
using our_struct =
  s2s::struct_field_list<
    s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
    s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
  >;

auto main() -> int {
  // Any input stream works; a std::ifstream over a file is the same call.
  std::stringstream input(
    std::string("\x05\x00\x00\x00\x00\x00\x00\x00" "hello", 13),
    std::ios::in | std::ios::binary);

  const auto res =
    s2s::struct_cast_le<our_struct>(input)
      .transform([](const our_struct& fields){
        std::println("len={} str={}", fields["len"_f], fields["str"_f]);
        return fields;
      }).transform_error([](const s2s::cast_error& err){
        std::println("failure_reason={} failed_at={}",
                     static_cast<int>(err.failure_reason), err.failed_at);
        return err;
      });

  return res ? 0 : 1;
}
```

The same schema drives the other direction. Fields the schema can work out for
itself — here `count`, the length of `data` — are derived during the write
rather than being yours to keep in sync:

<!-- docs: test/single_header/readme_roundtrip_example.cpp -->
```cpp
#include "s2s.hpp"

#include <sstream>
#include <vector>

using namespace s2s_literals;

using u16 = unsigned short;
using u32 = unsigned int;

// One schema, both directions.
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
  // "count" is never assigned. It is derived from data.size() during the write.

  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  if(const auto written = s2s::struct_write_be<our_struct>(stream, obj); !written)
    return 1;

  const auto back = s2s::struct_cast_be<our_struct>(stream);
  if(!back)
    return 1;

  return (*back)["count"_f] == 2
      && (*back)["data"_f] == std::vector<u16>{0x1122, 0x3344} ? 0 : 1;
}
```

## Documentation
[UserGuide.md](UserGuide.md) is the reference: the schema language and its four
axes, the constraint DSL, both directions in depth, writing a custom stream, and
what "compile time" does and does not mean here.

## Roadmap
- [x] Trivials
- [x] Arrays and vectors
- [x] Strings
- [x] Optionals
- [x] Unions
- [ ] Bitfields
- [ ] Read-Until Delimiter[s]
- [x] Support for all major compilers
- [ ] Full Support in Freestanding Compilers
- [x] Compile-time Endianness Handling
- [ ] Run-time Endianness Handling
- [ ] Modules
- [x] Write struct to stream
- [ ] struct_view - Zero copy views into buffers
- [ ] Asynchronous Read-Write
- [ ] Support for seeking
- [ ] Compile time functions for struct_field_list - 
    - [ ] Query fields
    - [ ] Extend with another list
    - [ ] Alignment and Padding Control
