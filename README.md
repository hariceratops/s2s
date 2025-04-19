
# struct_cast
A declarative binary parser to convert a stream into meta-struct which has a map like 
interface. Works extensively based on C++23 TMP. (The read in other direction is work-in-progress)

Library is single header and the file "struct_cast.hpp" from the single_header
folder can be used for direct inclusion into a project

## Features
* Single header
* Support for 
    * Trivial
    * Array of trivials 
    * Array of records 
    * Length prexied vector of trivials
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
struct_cast has a constraint on minimum version of the std to be C++23 with 
support for std::expected, std::byteswap. As of current implementation, 
unfortunately MSVC lacks support for std::endian and std::byteswap, 
hence struct_cast fails to compile, which leads to work-in-progress
for rolling them out

The compiler version requirements are 
* x86-64 gcc 13.1
* arm gcc 13.1
* arm64 gcc 13.1
* x86-64 clang 19.1.0
* armv8-a clang 19.1.0


## Taste of the API
```cpp
  #include "struct_cast.hpp"
  #include <print>

  using our_struct = 
    struct_field_list<
      basic_field<"len", std::size_t, field_size<fixed<8>>>,
      str_field<"str", field_size<len_from_field<"len">>>
    >;
  std::ifstream ifs("sample.bin", std::ios::in | std::ios::binary);
  auto res = struct_cast_le<our_struct>(ifs);
  auto fields = *res;
  std::println("len={} str={}", fields["len"_f], fields["str"_f]);
```

## API documentation
Refer file UserGuide.md for detailed API documentation, for a brief 
tour, refer the section [Brief Tour](Brief-Tour)

## Brief Tour
### "Struct"
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
template <typename field_accessor = /* field_lookup metafunction */>
    requires /* field_lookup success */
auto& operator[](field_accessor);

template <typename field_accessor = /* field_lookup metafunction */>
    requires /* field_lookup success */
const auto& operator[](field_accessor);
```

### Writing a data member schema
```cpp
using our_struct = 
  struct_field_list<
    basic_field<"len", std::size_t, field_size<fixed<8>>>,
    str_field<"str", field_size<len_from_field<"len">>>
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
- [x] Compile-time Endianness Handling
- [ ] Modules
- [ ] Run-time Endianness Handling
- [ ] Full Support in Freestanding Compilers
- [ ] Read-Until Delimiter[s]
- [ ] Support for all major compilers
- [ ] Write struct to stream
- [ ] struct_view - Zero copy views into buffers
- [ ] Asynchronous Read-Write
- [ ] Support for seeking
