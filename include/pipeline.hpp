#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_

#include <expected>
#include "error.hpp"


auto operator|(const read_result& res, auto&& callable) -> read_result
{
  return res ? callable() : std::unexpected(res.error());
}

#endif // _PIPELINE_HPP_
