#ifndef _STRUCT_CAST_IMPL_HPP_
#define _STRUCT_CAST_IMPL_HPP_

// status: split to cast and cast impl

#include <expected>
#include "../error/cast_error.hpp"
#include "../field/field.hpp"
#include "../field_list/field_list.hpp"
#include "../field_read/field_reader.hpp"
#include "../field_validation/field_value_constraints_traits.hpp"


namespace s2s {

constexpr auto operator|(const cast_result& res, auto&& callable) -> cast_result
{
  return res ? callable() : std::unexpected(res.error());
}


template <typename F, typename stream, auto endianness>
struct struct_cast_impl;

template <auto metadata, typename... fields, typename stream, auto endianness>
struct struct_cast_impl<struct_field_list_impl<metadata, fields...>, stream, endianness> {
  using S = struct_field_list_impl<metadata, fields...>;
  using R = std::expected<S, cast_error>;

  constexpr auto operator()(stream& s) -> R {
    S field_list;
    cast_result pipeline_seed{};
    auto res = (
      pipeline_seed |
      ... |
      [&]() -> cast_result {
        auto& field = static_cast<fields&>(field_list);
        auto reader = read_field<fields, S>(field, field_list);
        auto read_res = reader.template read<endianness>(s);
        // Short circuit the remaining pipeline since read failed for current field
        if(!read_res) {
          auto field_name = std::string_view{fields::field_id.data()};
          auto err = read_res.error();
          auto validation_err = cast_error{err, field_name};
          return std::unexpected(validation_err);
        }
        bool field_validation_res = fields::constraint_checker(field.value);
        if(!field_validation_res) {
          auto field_name = std::string_view{fields::field_id.data()};
          auto err = error_reason::validation_failure;
          auto validation_err = cast_error{err, field_name};
          return std::unexpected(validation_err);
        }
        // Both reading and validating went well
        return {};
      }
    );
    return res ? R(field_list) : std::unexpected(res.error());
  }
};

} /* namespace s2s */

#endif // _STRUCT_CAST_IMPL_HPP_
