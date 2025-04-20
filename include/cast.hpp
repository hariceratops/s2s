#ifndef _CAST_HPP_
#define _CAST_HPP_


#include <expected>
#include "field_reader.hpp"
#include "field_list.hpp"
#include "error.hpp"


namespace s2s {
template <typename T>
struct is_no_constraint;

template <typename T>
struct is_no_constraint<no_constraint<T>> {
  static constexpr bool res = true;
};

template <typename T>
struct is_no_constraint {
  static constexpr bool res = false;
};

template <typename T>
inline constexpr bool is_no_constraint_v = is_no_constraint<T>::res;


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
    read_result pipeline_seed{};
    auto res = (
      pipeline_seed |
      ... |
      [&]() -> read_result {
        auto& field = static_cast<fields&>(field_list);
        auto reader = read_field<fields, S>(field, field_list);
        auto read_res = reader.template read<endianness>(s);
        // Short circuit the remaining pipeline since read failed for current field
        if(!read_res) 
          return read_res;
        // Try validating with the constraint
        // todo enable check only if constraint is present, to avoid runtime costs?
        // if constexpr(is_no_constraint_v<decltype(fields::constraint_checker)>) {
          bool field_validation_res = fields::constraint_checker(field.value);
          if(!field_validation_res)
            return std::unexpected(cast_error::validation_failure);
        // }
        // Both reading and validating went well
        return {};
      }
    );
    return res ? R(field_list) : std::unexpected(res.error());
  }
};

template <s2s_input_stream_like stream_wrapper, field_list_like T, auto endianness>
constexpr auto struct_cast(stream_wrapper& wrapped) -> std::expected<T, cast_error> {
  return struct_cast_impl<T, stream_wrapper, endianness>{}(wrapped);
}

template <field_list_like T, input_stream_like stream>
constexpr auto struct_cast_le(stream& s) -> std::expected<T, cast_error> {
  using stream_wrapper = input_stream<stream>;
  stream_wrapper wrapped(s);
  return s2s::struct_cast_impl<T, stream_wrapper, std::endian::little>{}(wrapped);
}

template <field_list_like T, input_stream_like stream>
constexpr auto struct_cast_be(stream& s) -> std::expected<T, cast_error> {
  using stream_wrapper = input_stream<stream>;
  stream_wrapper wrapped(s);
  return s2s::struct_cast_impl<T, stream_wrapper, std::endian::big>{}(wrapped);
}
} /* namespace s2s */

#endif // _CAST_HPP_
