#ifndef _PIPELINE_HPP_
#define _PIPELINE_HPP_

#include <expected>
#include "error.hpp"


namespace s2s {
auto operator|(const read_result& res, auto&& callable) -> read_result
{
  return res ? callable() : std::unexpected(res.error());
}
} /* namespace s2s */


#endif // _PIPELINE_HPP_
