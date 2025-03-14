#ifndef _ERROR_HPP_
#define _ERROR_HPP_

#include <expected>

enum cast_error {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure
};


using read_result = std::expected<void, cast_error>;

#endif // _ERROR_HPP_
