#ifndef _FIELD__HPP_
#define _FIELD__HPP_


#include "compute_res.hpp"
#include "../field_constraints.hpp"
#include "../fixed_string.hpp"
#include "../field_base.hpp"

inline static constexpr auto always = [] { return true; };
using no_dependencies = typelist::typelist<>;

template <typename T>
concept field_containable = fixed_buffer_like<T> || arithmetic<T>;

template <fixed_string id,
          typename T,
          typename size_type,
          auto constraint_on_value = no_constraint<T>{},
          auto present_only_if = compute<always, no_dependencies>{},
          auto type_deducer = compute<always, no_dependencies>{}>
struct field: public field_base<id, T> {};

#endif // _FIELD__HPP_
