#ifndef _CLAUSE_HPP_
#define _CLAUSE_HPP_

#include "compute_res.hpp"


// todo constrain v to function like object returning bool
template <typename eval, typename T>
  requires is_eval_bool_from_fields_v<eval>
struct clause {
  static constexpr auto e = eval{};
  using type_tag = T;
};


template <typename T>
struct is_clause;

template <typename T>
struct is_clause {
  static constexpr bool res = false;
};

template <typename eval, typename T>
struct is_clause<clause<eval, T>> {
  static constexpr bool res = true;
};

template <typename T>
inline constexpr bool is_clause_v = is_clause<T>::res;

template <typename T>
concept clause_like = is_clause_v<T>;


#endif // _CLAUSE_HPP_
