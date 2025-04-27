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


## Errors
Error codes are returned in case a read fails. The read can fail 
currently in one of the three scenarios: field value validation failure,
provided input stream is exhausted or when type deduction failed while reading into union
```cpp
enum cast_error {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure
};
```
