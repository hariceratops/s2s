# The meta-struct

```cpp
template <typename... fields>
struct struct_field_list { /* implementation */ };
```

A `struct_field_list` is a meta-struct: a fixed set of named members with a
map-like interface. It is both the schema and the value. The same type that
declares what a format looks like is the type an instance of that format is
parsed into.

Members are reached with `operator[]` and the `_f` literal from
`s2s_literals`:

```cpp
frame["device_id"_f]
```

```cpp
template <typename field_accessor, typename field_lookup = /* field_lookup metafunction */>
    requires /* field_lookup success */
auto& operator[](field_accessor);

template <typename field_accessor, typename field_lookup = /* field_lookup metafunction */>
    requires /* field_lookup success */
const auto& operator[](field_accessor);
```

Lookup is a compile-time concept check, not a runtime search. Naming a field the
schema does not contain is a compilation failure, and so is declaring the same
id twice within one `struct_field_list`. There is no lookup cost at run time and
no way to ask for a field that is not there.

The non-const overload returns a mutable reference for ordinary fields and a
const reference for derived ones, which is how assigning to a derived field
becomes a compile error rather than a value that silently disagrees with the
data. [Writing](writing.md) covers which fields are derived and why.
