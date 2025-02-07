#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_

#include <expected>
#include "error.hpp"

// todo possible dead code
template <typename... expected_types>
auto is_any_error(const expected_types&... expected_list) {
  return (expected_list && ...);
}

// todo constraints
// todo fix the copying by using reference currently error thrown due to 
// non const lvalue being bound to rvalue
auto operator|(const read_result& res, auto&& callable) -> read_result
{
  return res ? callable() : std::unexpected(res.error());
}

#endif // _PIPELINE_HPP_
