#ifndef _COMPUTE_RES_
#define _COMPUTE_RES_

#include "typelist.hpp"
#include "struct_field_list_base.hpp"
#include <functional>
#include <optional>
#include "field_accessor.hpp"
#include "field_reader.hpp"


// todo add constriants
template <auto callable, typename... req_fields>
struct compute;

template <auto callable, typename... req_fields>
struct compute<callable, typelist::typelist<req_fields...>> {
  template <field_list_like flist>
  constexpr auto operator()(flist& field_list) {
    return std::invoke(callable, field_list[field_accessor<req_fields::field_id>{}]...);
  }
};

template <auto v, typename T>
struct match_case {
  static constexpr auto value = v;
  static constexpr auto reader = 
    [](T& dst, const unsigned char* buffer, std::size_t size_to_read) {
      read(dst, buffer, size_to_read);
    };
};

template <typename... Cases>
struct match_and_read;

template <>
struct match_and_read<> {
  constexpr void operator()(auto&, auto, const unsigned char*, std::size_t) const {
      // No match found, do nothing or handle the case
  }
};

// Recursive case: check head and proceed to tail
template <typename match_case_head, typename... match_case_rest>
struct match_and_read<match_case_head, match_case_rest...> {
  constexpr void operator()(auto& another, auto v, const unsigned char* buffer, std::size_t size_to_read) const {
    (v == match_case_head::value ? match_case_head::reader(another, buffer, size_to_read)
                                 : match_and_read<match_case_rest...>()(another, v, buffer, size_to_read));
  }
};

#endif // _COMPUTE_RES_
