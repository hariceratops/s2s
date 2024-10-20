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

#endif // _CLAUSE_HPP_
