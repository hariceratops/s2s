#ifndef _CAST_HPP_
#define _CAST_HPP_


#include <expected>
#include "field_reader.hpp"
// #include "field_meta.hpp"
#include "field_list.hpp"
#include "error.hpp"


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
      [&](){
        auto reader = read_field<fields, S>(static_cast<fields&>(field_list), field_list);
        return reader.template read<endianness>(s);
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
  return struct_cast_impl<T, stream_wrapper, std::endian::little>{}(wrapped);
}

template <field_list_like T, input_stream_like stream>
constexpr auto struct_cast_be(stream& s) -> std::expected<T, cast_error> {
  using stream_wrapper = input_stream<stream>;
  stream_wrapper wrapped(s);
  return struct_cast_impl<T, stream_wrapper, std::endian::big>{}(wrapped);
}

#endif // _CAST_HPP_
