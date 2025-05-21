#ifndef _CAST_ERROR_HPP_
#define _CAST_ERROR_HPP_


#include <expected>
#include <string_view>

namespace s2s {
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure
};


struct cast_error {
  error_reason failure_reason;
  std::string_view failed_at;
};


using rw_result = std::expected<void, error_reason>;
using cast_result = std::expected<void, cast_error>;

} /* namespace s2s */

#endif // _CAST_ERROR_HPP_
