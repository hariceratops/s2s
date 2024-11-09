#ifndef _CAST_HPP_
#define _CAST_HPP_


#include <expected>
#include "field_reader.hpp"
#include "sc_type_traits.hpp"
#include "field_traits.hpp"
#include "struct_field_list_traits.hpp"
#include "field_meta.hpp"
#include "field_list.hpp"

// todo possible dead code
template <typename... expected_types>
auto is_any_error(const expected_types&... expected_list) {
  return (expected_list && ...);
}

// todo constraints
// todo fix the copying by using reference currently error thrown due to 
// non const lvalue being bound to rvalue
template <typename expected_struct_field_list, typename error>
auto operator|(std::expected<expected_struct_field_list, error> lhs, auto functor) {
  if(lhs) return functor(*lhs); else return lhs;
}

// todo constraint with struct field list else user might have to provide all
// fields as params to the function
template <typename T>
struct struct_cast_impl;

template <typename... fields>
struct struct_cast_impl<struct_field_list<fields...>> {
  using field_list_type = struct_field_list<fields...>;
  using res_type = std::expected<struct_field_list<fields...>, std::string>;

  constexpr auto operator()(const unsigned char* buffer) -> res_type {
    res_type res = field_list_type{};
    std::size_t prefix_sum[sizeof...(fields) + 1] = {0};
    std::size_t index = 0;

    return (
      res | ... | ([&prefix_sum, &buffer, &index](field_list_type input) -> res_type {
        using field_size = typename fields::field_size;
        using field_type = typename fields::field_type;

        auto field = static_cast<fields&>(input);
        std::expected<field_type, std::string> field_value;
        auto buffer_pos = reinterpret_cast<const char*>(buffer + prefix_sum[index]);

        if constexpr (is_optional_field_v<field_type>) {
          // todo variable length optional field
          field_value = field_type::field_presence_checker(input) ? 
                        read<field_type>(buffer_pos, field_type::field_size) :
                        res_type{std::nullopt};
        } else if constexpr (is_union_field_v<field_type>) {
          using variant_reader_type_t = variant_reader<field_type, field_size>;
          auto type_index = field_type::type_deduction_guide(input); 
          field_value = *type_index ? 
                        variant_reader_type_t{}(type_index, buffer_pos): 
                        type_index;
        } else if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
          field_value = struct_cast(field.value, buffer_pos);
        } else if constexpr (is_field_v<field_type>) {
          field_value = read<field_type>(buffer_pos, field_size::size::N);
        } else if constexpr (is_runtime_sized_field_v<field_type>) {
          field_value = read<field_type>(buffer_pos, input[field_type::field_accessor]);
        }

        // todo fix bug in case of updating the prefic sum for var len field
        // also handle optional field, both read and unread case and variant read
        // some func or deduction to get size out of type_size
        prefix_sum[index + 1] = prefix_sum[index] + 0;
        ++index;
        
        // todo return std::unexpected to break the pipeline
        // is this ok?
        if(field_value) field.value = *field_value;
        // currently compile error
        // else input = field_value;
        
        // todo constraint checker
        // static constexpr auto constraint_checker = constraint_on_value;
        return input;
      }
    ));
  }
};

template <typename T>
  requires is_struct_field_list_v<T>
constexpr auto struct_cast(const unsigned char* buffer) -> std::expected<T, std::string> {
  return struct_cast_impl<T>{}(buffer);
}

template <typename... fields>
constexpr void struct_cast(struct_field_list<fields...>& field_list, std::ifstream& stream) {
  ([&](auto& field) {
    using field_type = std::decay_t<decltype(field)>;
    if constexpr (is_struct_field_list_v<extract_type_from_field_v<field_type>>) {
      struct_cast(field.value, stream);
    } else if constexpr (is_field_v<field_type>) {
      field.read(stream, field_type::field_size);
    } else if constexpr (is_runtime_sized_field_v<field_type>) {
      field.read(stream, field_list[field_type::field_accessor]);
    }

  }(static_cast<fields&>(field_list)), ...);
}

#endif // _CAST_HPP_
