
# s2s
A declarative binary parser aka serde to convert a stream into meta-struct which has a map like 
interface, and back again.

Implemented as an embedded DSL powered by C++23 TMP.

**The parser is generated at compile time.** The schema is declared as a type
and the template machinery turns it into straight-line read and write code —
no runtime dispatch over the field list, no reflection, no format interpreter.
This is unconditional. It holds when the stream is a plain `std::ifstream` and
when the schema is full of `std::string` and `std::vector`, and it comes with
using the library at all.

**Parsing can additionally be performed at compile time.** This is the narrower
claim, and it has two conditions: the stream must be usable during constant
evaluation, and the parsed value must not carry heap memory out of it. When both
hold, the whole parse — validation included — happens in the compiler and the
result is a `constexpr` value. When they do not, everything in the paragraph
above still holds.

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
* Lengths and union discriminants derived on write, so they cannot drift from
  the data they describe — a length is not part of the struct's interface at
  all, and a discriminant is read-only
* Validation of fields in place while reading and while writing
* A per-field ceiling on what a wire-supplied length may allocate, on by
  default, so a corrupt stream cannot ask for gigabytes
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
<!-- docs: test/doc_examples/readme_read_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <print>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// A firmware image: a two-byte marker, a version, and a payload whose length
// is carried on the wire just before it.
using firmware_image =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x46, 0x57}>,
    s2s::basic_field<"version", u16, 2_B>,
    s2s::basic_field<"payload_length", u32, 4_B>,
    s2s::vec_field<"payload", u8, s2s::len_from_field<"payload_length">>
  >;

auto main() -> int {
  std::ifstream image("firmware.bin", std::ios::in | std::ios::binary);

  const auto res =
    s2s::struct_cast_be<firmware_image>(image)
      .transform([](const firmware_image& fields){
        std::println("version={} payload={} bytes",
                     fields["version"_f], fields["payload"_f].size());
        return fields;
      }).transform_error([](const s2s::cast_error& err){
        std::println("failure_reason={} failed_at={}",
                     static_cast<int>(err.failure_reason), err.failed_at);
        return err;
      });

  return res.has_value() ? 0 : 1;
}
```

The same schema drives the other direction. Fields the schema can work out for
itself — here `payload_length` — are derived during the write rather than being
data anyone has to keep in sync, which is why the schema names it and the
struct does not expose it:

<!-- docs: test/doc_examples/readme_roundtrip_example.cpp -->
```cpp
#include "s2s.hpp"

#include <array>
#include <fstream>
#include <vector>

using namespace s2s_literals;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

// One schema, both directions.
using firmware_image =
  s2s::struct_field_list<
    s2s::magic_byte_array<"marker", 2, std::array<u8, 2>{0x46, 0x57}>,
    s2s::basic_field<"version", u16, 2_B>,
    s2s::basic_field<"payload_length", u32, 4_B>,
    s2s::vec_field<"payload", u8, s2s::len_from_field<"payload_length">>
  >;

auto main() -> int {
  firmware_image image{};
  image["version"_f] = u16{1};
  image["payload"_f] = std::vector<u8>{0xde, 0xad, 0xbe, 0xef};
  // "payload_length" cannot be named at all. It is derived from payload.size().

  std::fstream file("firmware_out.bin",
                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

  // Each step returns an expected, so writing and reading back chain into one
  // expression; a failure at either end carries its cast_error through.
  const auto round_tripped =
    s2s::stream_cast_be<firmware_image>(file, image)
      .and_then([&file] {
        // A file stream shares one position between reads and writes.
        file.seekg(0);
        return s2s::struct_cast_be<firmware_image>(file);
      })
      .transform([](const firmware_image& parsed) {
        return parsed["payload"_f].size() == 4;
      });

  return round_tripped.value_or(false) ? 0 : 1;
}
```

## Documentation
The full guide lives in [`docs/`](docs/index.md) and is published as a site
built with mkdocs-material: the schema language and its four axes, the
constraint DSL, both directions in depth, writing a custom stream, and what
"compile time" does and does not mean here.

### Reading it locally

Everything needed to render the guide lives in `docs/`, so both commands name
the config there:

```sh
pip install -r docs/requirements.txt

mkdocs serve -f docs/mkdocs.yml   # live-reloading site on http://127.0.0.1:8000
mkdocs build -f docs/mkdocs.yml   # or render once into site/
```

`mkdocs serve` watches the pages and reloads on every edit. They are plain
markdown with relative links, so they also read fine straight from
[`docs/`](docs/index.md) on GitHub — building is only needed for the rendered
site.

## Building and running the tests

Configure and build. gcc 14 or newer is required — `CMakeLists.txt` fails
configure below it rather than letting the failure surface later as a template
error:

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j$(nproc)
ctest --test-dir build -j$(nproc)
```

GoogleTest and qlibs/ut are both fetched at configure time, so the first
configure needs network access. Nothing else is vendored.

A single suite, or everything matching a name:

```sh
ctest --test-dir build -R union            # every union entry, both frameworks
ctest --test-dir build -R '_ct_'           # the compile-time tier only
ctest --test-dir build --output-on-failure # show output from what failed
```

The test tree is named for what each directory verifies:

| Directory | Verifies |
|---|---|
| `test/schema/` | one schema construct per file pair — the descriptor table in [`docs/schema/`](docs/schema/index.md) |
| `test/internals/` | traits, containers and field-list metadata |
| `test/must_not_compile/` | programs that must fail to compile |
| `test/doc_examples/` | every program shown in the docs compiles, runs, and matches its fenced block |
| `test/shipped_header/` | the amalgamated header stands alone and regenerates byte-identically |
| `examples/` | standalone programs a consumer could copy, built and run |

Tests come in two forms. `<feature>_read.cpp` and `<feature>_write.cpp` are
GoogleTest and run at run time. `<feature>_read_ct.cpp` and `<feature>_write_ct.cpp`
are [qlibs/ut](https://github.com/qlibs/ut) and run **both** at compile time and
at run time, from the same source — which is how the compile-time parsing claim
above is checked rather than asserted. Each ut source produces three CTest
entries:

| Entry | What it does |
|---|---|
| `<name>_compile_time` | drives `cmake --build`; the assertions are spent during compilation, so a failure is a build failure |
| `<name>_run_time` | runs the same tests at run time, printing actual values — the one build that still links when a compile-time expectation fails |
| `<name>_coverage` | fails unless *every* test also ran at compile time; ut skips a capturing lambda silently, and this is what notices |

Entries under `test/must_not_compile/` are expected to fail to build, and
`ctest` reports them green when they do.

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
