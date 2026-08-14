#ifndef _COMPTIME_FIELD_SIZE_DEDUCE_HPP_
#define _COMPTIME_FIELD_SIZE_DEDUCE_HPP_


#include "../field_size/field_size.hpp"


namespace s2s {
// The one value-keyed template on the size axis: byte_count carries its width
// in the value, so this reads the value rather than the type. The variable
// size case lives in field_size_deduce.hpp, which this header must not pull
// in — it would drag field_list.hpp along with it.
template <auto size>
struct deduce_field_size;

template <auto size>
  requires fixed_size_like<size_type_of<size>>
struct deduce_field_size<size> {
  constexpr auto operator()() const -> std::size_t {
    return size.count;
  }
};
} /* namespace s2s */


#endif // _COMPTIME_FIELD_SIZE_DEDUCE_HPP_
