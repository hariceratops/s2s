
# s2s
A declarative binary parser aka serde to convert a stream into meta-struct which has a map like 
interface.

(The read in other direction is work-in-progress)

Implemented as an embedded DSL powered by C++23 TMP.

Library is single header and the file "s2s.hpp" from the single_header
folder can be used for direct inclusion into a project

## Features
* Single header
* constexpr! as much as possible
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
* Validation of fields in place while reading
* Compile time endianness handling 
* Pluggable interfaces working with custom streams

## Requirements
s2s currently has a constraint on minimum version of the std to be C++23

The compiler version requirements are 
* gcc 13.1 : x86-64, arm, arm64 gcc 13.1
* clang 19.1.0 : x86-64, armv8-a
* msvc v19.39, VS 17.9 : x64, x86, arm64


## Taste of the API
Link to Godbolt: https://godbolt.org/z/eGTvdoejP
```cpp
  #include "s2s.hpp"
  #include <print>

  using namespace s2s_literals;

  auto main(void) -> int {
    // Our "struct" has 2 members. A length field of size 8 and type
    // std::size_t, and a length prefixed string whose length is 
    // derived from the "len" field
    using our_struct = 
      s2s::struct_field_list<
        s2s::basic_field<"len", std::size_t, s2s::field_size<s2s::fixed<8>>>,
        s2s::str_field<"str", s2s::field_size<s2s::len_from_field<"len">>>
      >;
    std::ifstream ifs("sample.bin", std::ios::in | std::ios::binary);
    auto res = 
      s2s::struct_cast_le<our_struct>(ifs)
          .transform([](const our_struct& fields){
            std::println("len={} str={}", fields["len"_f], fields["str"_f]);
            return fields;
          }).transform_error([](const s2s::cast_error& err){
            std::println("failure_reason={} failed_at={}", static_cast<int>(err.failure_reason), err.failed_at);
            return err;
          });
    return 0;
  }
```

Or let's go constexpr everything, as long as we do not have fields which we would
allocate, say vector or string. 
Link to Godbolt: https://godbolt.org/z/G5GWTEEq3
```cpp
  #include "s2s.hpp"
  #include "test/constexpr_memstream.hpp"
  #include <print>

  using namespace s2s_literals;

  using u32 = unsigned int;
  // a trivial struct with two u32 members
  using our_struct =
    s2s::struct_field_list<
      s2s::basic_field<"a", u32, s2s::field_size<s2s::fixed<4>>>,
      s2s::basic_field<"b", u32, s2s::field_size<s2s::fixed<4>>>
    >;

  constexpr auto parse_our_struct() -> std::expected<our_struct, s2s::cast_error>
  {
    std::array<u8, 8> buffer{0xef, 0xbe, 0xad, 0xde, 0x0d, 0xd0, 0xfe, 0xca};
    // custom stream written for compile time struct_cast
    // refer constexpr_memstream.hpp for implementation which can 
    // be extended as required
    memstream<8> stream(buffer);
    return s2s::struct_cast_le<our_struct>(stream);
  }

  // complete marshalling and validation in compile time
  constexpr auto res = parse_our_struct();
  static_assert(res);
  constexpr auto fields = *res;
  static_assert(fields["a"_f] == 0xdeadbeef);
  static_assert(fields["b"_f] == 0xcafed00d);

  auto main(void) -> int {
    // Compiler might emit assembly for [] operators
    // We can further optimize to store [] operator results
    // in constexpr variable
    std::println("{} {}", fields["a"_f], fields["b"_f]);
    return 0;
  }
```

## API documentation
Refer file UserGuide.md for detailed API documentation, for a brief 
tour, refer the section [Brief Tour](#Brief-Tour)

## Brief Tour
### "Struct"
```cpp
template <typename... fields>
struct struct_field_list { /* implementation */ };
```
A struct_field_list datatype is a meta-struct with a map-like interface to access
a field member for read or write operation. The operator[] when provided with 
custom literal "<field_name>"_f as key, returns a reference or const reference 
to the member with name <field_name>

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

### Writing a data member schema
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

### Cast API
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

Error codes are returned in case a read fails. The read can fail 
currently in one of the three scenarios: field value validation failure,
provided input stream is exhausted or when type deduction failed while reading into union


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
- [ ] Write struct to stream
- [ ] struct_view - Zero copy views into buffers
- [ ] Asynchronous Read-Write
- [ ] Support for seeking
- [ ] Compile time functions for struct_field_list - 
    - [ ] Query fields
    - [ ] Extend with another list
    - [ ] Alignment and Padding Control
