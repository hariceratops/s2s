#ifndef _CAST_ERROR_HPP_
#define _CAST_ERROR_HPP_

#include <expected>

namespace s2s {
enum cast_error {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure
};


using read_result = std::expected<void, cast_error>;
} /* namespace s2s */

#endif // _CAST_ERROR_HPP_
