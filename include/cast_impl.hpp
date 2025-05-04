#ifndef _CAST_IMPL_HPP_
#define _CAST_IMPL_HPP_


#include <expected>
#include "cast_error.hpp"
#include "field.hpp"
#include "field_list.hpp"
#include "stream_wrapper_impl.hpp"
#include "field_reader.hpp"
#include "field_value_constraints_traits.hpp"


namespace s2s {

constexpr auto operator|(const cast_result& res, auto&& callable) -> cast_result
{
  return res ? callable() : std::unexpected(res.error());
}

// forward declaration
template <s2s_input_stream_like stream, field_list_like T, auto endianness>
constexpr auto struct_cast(stream&) -> std::expected<T, cast_error>;

template <typename F, typename stream, auto endianness>
struct struct_cast_impl;

template <typename... fields, typename stream, auto endianness>
struct struct_cast_impl<struct_field_list<fields...>, stream, endianness> {
  using S = struct_field_list<fields...>;
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
          auto field_name = std::string{fields::field_id.data()};
          auto err = read_res.error();
          auto validation_err = cast_error{err, field_name};
          return std::unexpected(validation_err);
        }
        // Try validating with the constraint
        // todo enable check only if constraint is present, to avoid runtime costs?
        // if constexpr(is_no_constraint_v<decltype(fields::constraint_checker)>) {
          bool field_validation_res = fields::constraint_checker(field.value);
          if(!field_validation_res) {
            auto field_name = std::string{fields::field_id.data()};
            auto err = error_reason::validation_failure;
            auto validation_err = cast_error{err, field_name};
            return std::unexpected(validation_err);
          }
        // }
        // Both reading and validating went well
        return {};
      }
    );
    return res ? R(field_list) : std::unexpected(res.error());
  }
};

template <s2s_input_stream_like stream_wrapper, field_list_like T, auto endianness>
[[nodiscard]] constexpr auto struct_cast(stream_wrapper& wrapped) -> std::expected<T, cast_error> {
  return struct_cast_impl<T, stream_wrapper, endianness>{}(wrapped);
}

template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_le(stream& s) -> std::expected<T, cast_error> {
  using stream_wrapper = input_stream<stream>;
  stream_wrapper wrapped(s);
  return struct_cast_impl<T, stream_wrapper, std::endian::little>{}(wrapped);
}

template <field_list_like T, input_stream_like stream>
[[nodiscard]] constexpr auto struct_cast_be(stream& s) -> std::expected<T, cast_error> {
  using stream_wrapper = input_stream<stream>;
  stream_wrapper wrapped(s);
  return struct_cast_impl<T, stream_wrapper, std::endian::big>{}(wrapped);
}
} /* namespace s2s */

#endif // _CAST_IMPL_HPP_
