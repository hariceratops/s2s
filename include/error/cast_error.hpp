#ifndef _CAST_ERROR_HPP_
#define _CAST_ERROR_HPP_


#include <expected>
#include <string_view>

namespace s2s {
enum error_reason {
  buffer_exhaustion,
  validation_failure,
  type_deduction_failure,
  // Two parts of the struct imply different lengths for the same data — a
  // cross-field disagreement, not a value that is wrong on its own terms.
  // Appended rather than inserted so the existing enumerators keep their
  // values.
  found_contradicting_length
};


struct cast_error {
  error_reason failure_reason;
  std::string_view failed_at;
};


using rw_result = std::expected<void, error_reason>;
using cast_result = std::expected<void, cast_error>;


// Both directions fold their per-field steps through these, so they live here
// rather than in the read path's headers — the amalgamated header is a single
// translation unit, where a second definition would be an error.
constexpr auto operator|(const cast_result& res, auto&& callable) -> cast_result
{
  return res ? callable() : std::unexpected(res.error());
}

constexpr auto operator|(const rw_result& res, auto&& callable) -> rw_result
{
  return res ? callable() : std::unexpected(res.error());
}

} /* namespace s2s */

#endif // _CAST_ERROR_HPP_
