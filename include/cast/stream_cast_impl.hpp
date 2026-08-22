#ifndef _STREAM_CAST_IMPL_HPP_
#define _STREAM_CAST_IMPL_HPP_


#include <expected>
#include "../error/cast_error.hpp"
#include "../field_list/field_list.hpp"
#include "../field_write/field_writer.hpp"


namespace s2s {

template <typename F, typename stream, auto endianness>
struct stream_cast_impl;

template <auto metadata, typename... fields, typename stream, auto endianness>
struct stream_cast_impl<struct_field_list_impl<metadata, fields...>, stream, endianness> {
  using S = struct_field_list_impl<metadata, fields...>;

  constexpr auto operator()(stream& s, const S& field_list) -> cast_result {
    cast_result pipeline_seed{};
    return (
      pipeline_seed |
      ... |
      [&]() -> cast_result {
        const auto& field = static_cast<const fields&>(field_list);
        // Validated before writing, not after: a struct that fails its own
        // constraint would otherwise emit bytes that cannot be read back.
        // Fields whose stored value the write path ignores are the exception —
        // a derived one has its constraint checked against the derived value
        // instead, and a frozen one takes its value from the constraint and so
        // cannot fail it. Neither is ever assigned, so the stored value here is
        // whatever default construction left behind.
        if constexpr(!is_derived_target_v<fields, S> && !is_frozen_target_v<fields, S>) {
          if(!fields::constraint_checker(field.value)) {
            auto field_name = std::string_view{fields::field_id.data()};
            return std::unexpected(cast_error{error_reason::validation_failure, field_name});
          }
        }
        auto writer = write_field<fields, S>(field.value, field_list);
        auto write_res = writer.template write<endianness>(s);
        if(!write_res) {
          auto field_name = std::string_view{fields::field_id.data()};
          return std::unexpected(cast_error{write_res.error(), field_name});
        }
        return {};
      }
    );
  }
};

} /* namespace s2s */

#endif // _STREAM_CAST_IMPL_HPP_
